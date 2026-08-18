#include <Server/Client.hh>

#include <Server/Game.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>

#include <Helpers/UTF8.hh>
#include <sstream>
#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <cmath>

#include <array>
#include <iostream>
constexpr std::array<uint32_t, RarityID::kNumRarities> RARITY_TO_XP = {2, 10, 50, 200, 1000, 5000, 0};

Client::Client() : game(nullptr)
{
}

void Client::init()
{
    DEBUG_ONLY(assert(game == nullptr);)
    Server::game.add_client(this);
}

void Client::remove()
{
    if (game == nullptr) return;
    game->remove_client(this);
}

void Client::disconnect(int reason, std::string const& message)
{
    if (ws == nullptr) return;
    remove();
    ws->end(reason, message);
}

uint8_t Client::alive()
{
    if (game == nullptr) return false;
    Simulation* simulation = &game->simulation;
    return simulation->ent_exists(camera)
        && simulation->ent_exists(simulation->get_ent(camera).get_player());
}

void Client::on_message(WebSocket* ws, std::string_view message, uint64_t code)
{
    if (ws == nullptr) return;
    uint8_t const* data = reinterpret_cast<uint8_t const*>(message.data());
    Reader reader(data);
    Validator validator(data, data + message.size());
    Client* client = ws->getUserData();
    if (client == nullptr)
    {
        ws->end(CloseReason::kServer, "Server Error");
        return;
    }
    if (!client->verified)
    {
        if (client->check_invalid(validator.validate_uint8() && validator.validate_uint64())) return;
        if (reader.read<uint8_t>() != Serverbound::kVerify)
        {
            client->disconnect();
            return;
        }
        if (reader.read<uint64_t>() != VERSION_HASH)
        {
            client->disconnect(CloseReason::kOutdated, "Outdated Version");
            return;
        }
        client->verified = 1;
        client->init();
        return;
    }
    if (client->game == nullptr)
    {
        client->disconnect();
        return;
    }
    if (client->check_invalid(validator.validate_uint8())) return;
    switch (reader.read<uint8_t>())
    {
    case Serverbound::kVerify:
        client->disconnect();
        return;
    case Serverbound::kClientInput:
        {
            if (!client->alive()) break;
            Simulation* simulation = &client->game->simulation;
            Entity& camera = simulation->get_ent(client->camera);
            Entity& player = simulation->get_ent(camera.get_player());
            if (client->check_invalid(
                validator.validate_float() &&
                validator.validate_float() &&
                validator.validate_uint8()
            ))
                return;
            float x = reader.read<float>();
            float y = reader.read<float>();
            // Reject NaN/Inf outright. Comparisons against NaN are always false,
            // so `x == 0` and `|x| > 5e3` alone would let a NaN payload slip
            // straight into player.acceleration and then into velocity/position
            // via tick_entity_motion. is_finite catches it.
            if (!is_finite(x) || !is_finite(y)) break;
            if (std::abs(x) > 5e3 || std::abs(y) > 5e3) break;
            if (x == 0 && y == 0) player.acceleration.set(0, 0);
            else
            {
                Vector accel(x, y);
                float m = accel.magnitude();
                if (m > 200) accel.set_magnitude(PLAYER_ACCELERATION);
                else accel.set_magnitude(m / 200 * PLAYER_ACCELERATION);
                player.acceleration = accel;
            }
            player.input = reader.read<uint8_t>();
            break;
        }
    case Serverbound::kClientSpawn:
        {
            if (client->alive()) break;
            //check string length
            std::string name;
            if (client->check_invalid(validator.validate_string(MAX_NAME_LENGTH))) return;
            reader.read<std::string>(name);
            if (client->check_invalid(UTF8Parser::is_valid_utf8(name))) return;
            Simulation* simulation = &client->game->simulation;
            Entity& camera = simulation->get_ent(client->camera);
            Entity& player = alloc_player(simulation, camera.get_team());
            player_spawn(simulation, camera, player);
            player.set_name(name);
            break;
        }
    case Serverbound::kPetalDelete:
        {
            if (!client->alive()) break;
            Simulation* simulation = &client->game->simulation;
            Entity& camera = simulation->get_ent(client->camera);
            Entity& player = simulation->get_ent(camera.get_player());
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t pos = reader.read<uint8_t>();
            if (pos >= MAX_SLOT_COUNT + player.get_loadout_count()) break;
            PetalID::T old_id = player.get_loadout_ids(pos);
            RarityID::T old_rarity = player.get_loadout_rarities(pos);
            if (old_id != PetalID::kNone && old_id != PetalID::kBasic)
            {
                uint8_t rarity = old_rarity;
                player.set_score(player.get_score() + RARITY_TO_XP[rarity]);
                //need to delete if over cap
                if (player.deleted_petals.size() == player.deleted_petals.capacity())
                    //removes old trashed old petal
                    PetalTracker::remove_petal(simulation, player.deleted_petals[0]);
                player.deleted_petals.push_back(old_id);
            }
            player.set_loadout_ids(pos, PetalID::kNone);
            player.set_loadout_rarities(pos, RarityID::kCommon); //TODO aaa
            break;
        }
    case Serverbound::kPetalSwap:
        {
            if (!client->alive()) break;
            Simulation* simulation = &client->game->simulation;
            Entity& camera = simulation->get_ent(client->camera);
            Entity& player = simulation->get_ent(camera.get_player());
            if (client->check_invalid(validator.validate_uint8() && validator.validate_uint8())) return;
            uint8_t pos1 = reader.read<uint8_t>();
            if (pos1 >= MAX_SLOT_COUNT + player.get_loadout_count()) break;
            uint8_t pos2 = reader.read<uint8_t>();
            if (pos2 >= MAX_SLOT_COUNT + player.get_loadout_count()) break;
            PetalID::T tmp_id = player.get_loadout_ids(pos1);
            RarityID::T tmp_rarity = player.get_loadout_rarities(pos1);
            player.set_loadout_ids(pos1, player.get_loadout_ids(pos2));
            player.set_loadout_rarities(pos1, player.get_loadout_rarities(pos2));
            player.set_loadout_ids(pos2, tmp_id);
            player.set_loadout_rarities(pos2, tmp_rarity);
            break;
        }
    case Serverbound::kChatSend:
        {
            if (!client->alive()) break;
            Simulation* simulation = &client->game->simulation;
            Entity& camera = simulation->get_ent(client->camera);
            Entity& player = simulation->get_ent(camera.get_player());
            std::string text;
            if (client->check_invalid(validator.validate_string(MAX_CHAT_LENGTH))) return;
            reader.read<std::string>(text);
            if (client->check_invalid(UTF8Parser::is_valid_utf8(text))) return;
            text = UTF8Parser::trunc_string(text, MAX_CHAT_LENGTH);
            if (text.empty()) break;

            if (client->check_invalid(validator.validate_uint8() && validator.validate_float())) return;
            float x = reader.read<float>();
            float y = reader.read<float>();

            if (text[0] == '/')
            {
                command(client, text.substr(1), x, y);
                break;
            }

            client->game->chat(player.id, text);
            break;
        }
    }
}

void Client::command(Client* client, std::string const& text, float client_x, float client_y)
{
    Simulation* simulation = &client->game->simulation;
    Entity& camera = simulation->get_ent(client->camera);
    Entity& player = simulation->get_ent(camera.get_player());

    std::string lower_text = text;
    std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);

    std::istringstream iss(lower_text);
    std::string command, arg;
    iss >> command;

    float x = player.get_x();
    float y = player.get_y();
    if (command.ends_with("to")) {
        command = command.substr(0, command.size() - 2);  // 去掉 "to" 后缀
        x += client_x;
        y += client_y;
    }

    if (command == "kill")
    {
        simulation->get_ent(player.get_parent()).set_killed_by(player.get_name());
        simulation->request_delete(player.id);
    }
    if (command == "drop" || command == "give") {
        std::string arg_id, arg_rarity;  // 分别读取两个字符串
        if (iss >> arg_id >> arg_rarity) {
            try {
                int id_int = std::stoi(arg_id);
                int rarity_int = std::stoi(arg_rarity);

                // 校验 ID
                if (id_int < 0 || id_int >= PetalID::kNumPetals) {
                    // 输出错误信息，例如 "Invalid petal ID"
                    return; // 或 continue
                }
                // 校验稀有度（假设有范围）
                if (rarity_int < 0 || rarity_int >= RarityID::kNumRarities) {
                    // 输出错误信息
                    return;
                }

                PetalID::T id = static_cast<PetalID::T>(id_int);
                RarityID::T rarity = static_cast<RarityID::T>(rarity_int);

                // 调用你现在的 alloc_drop（接收 id 和 rarity）
                Entity& drop = alloc_drop(simulation, id, rarity);
                drop.set_x(x), drop.set_y(y);

            } catch (const std::invalid_argument&) {
                // 参数不是数字
                // 输出用法提示
            } catch (const std::out_of_range&) {
                // 数字超出范围
            }
        } else {
            // 参数数量不足
            // 例如: "Usage: /give <petalID> <rarity>"
        }
    }
    /*
    else if (command == "bbht") {
        std::vector<PetalID::T> fixed_loadout = {
           PetalID::kDahlia,
           PetalID::kSalt,
           PetalID::kDahlia,
           PetalID::kBubble,
           PetalID::kStinger,
           PetalID::kIris,
           PetalID::kStinger,
           PetalID::kDandelion,
        };

        for (uint32_t i = 0; i < fixed_loadout.size(); ++i) {
            PetalID::T pid = fixed_loadout[i];
            player.set_inventory(i, pid);
            LoadoutSlot& slot = player.loadout[i];
            player.set_loadout_ids(i, pid);
            slot.update_id(simulation, pid);
            slot.force_reload();
        }
        for (uint32_t i = 0; i < loadout_slots_at_level(30); ++i)
            PetalTracker::add_petal(simulation, player.get_inventory(i));
    }
    if (!client->isAdmin) return;

    if (command == "drop" || command == "give") {
        PetalID::T id;
        while (iss >> arg) {
            try { id = std::stoi(arg); }
            catch (const std::invalid_argument&) { continue; }
            catch (const std::out_of_range&) { continue; }
            if (id >= PetalID::kNumPetals) continue;
            Entity& drop = alloc_drop(simulation, id);
            drop.set_x(player.get_x()), drop.set_y(player.get_y());
        }
    }
    else if (command == "dropto") {
        PetalID::T id;
        while (iss >> arg) {
            try { id = std::stoi(arg); }
            catch (const std::invalid_argument&) { continue; }
            catch (const std::out_of_range&) { continue; }
            if (id >= PetalID::kNumPetals) continue;
            Entity& drop = alloc_drop(simulation, id);
            drop.set_x(x), drop.set_y(y);
        }
    }
    */
    else if (command == "tp")
    {
        try { iss >> arg, x = std::stof(arg), iss >> arg, y = std::stof(arg); }
        catch (const std::invalid_argument&) {}
        catch (const std::out_of_range&) {}
        if (!is_finite(x) || !is_finite(y)) return;
        player.set_x(fclamp(x, 0, ARENA_WIDTH)), player.set_y(fclamp(y, 0, ARENA_HEIGHT));
    }
    else if (command == "xp")
    {
        uint32_t xp;
        try { iss >> arg, xp = uint32_t(std::stoul(arg)); }
        catch (const std::invalid_argument&) { return; }
        catch (const std::out_of_range&) { return; }
        player.set_score(player.get_score() + xp);
    }
    else if (command == "spawn")
    {
        std::string arg_id, arg_rarity;
        if (iss >> arg_id >> arg_rarity)
        {
            try
            {
                int id_int = std::stoi(arg_id);
                int rarity_int = std::stoi(arg_rarity);
                if (id_int < 0 || id_int >= MobID::kNumMobs)
                {
                    return;
                }
                if (rarity_int < 0 || rarity_int >= RarityID::kNumRarities)
                {
                    return;
                }
                MobID::T id = static_cast<MobID::T>(id_int);
                RarityID::T rarity = static_cast<RarityID::T>(rarity_int);
                alloc_mob(simulation, id, rarity, x, y, NULL_ENTITY);
            }
            catch (const std::invalid_argument&) { return; }
            catch (const std::out_of_range&) { return; }

        }
        else
        {

        }
    }
    /*
    else if (command == "spawnto")
    {
        MobID::T id;
        while (iss >> arg)
        {
            try { id = std::stoi(arg); }
            catch (const std::invalid_argument&) { continue; }
            catch (const std::out_of_range&) { continue; }
            if (id >= MobID::kNumMobs) continue;
            alloc_mob(simulation, id, x, y, NULL_ENTITY);
        }
    }

    else if (command == "spawnally")
    {
        MobID::T id;
        while (iss >> arg)
        {
            try { id = std::stoi(arg); }
            catch (const std::invalid_argument&) { continue; }
            catch (const std::out_of_range&) { continue; }
            if (id >= MobID::kNumMobs) continue;
            alloc_mob(simulation, id, player.get_x(), player.get_y(), player.get_team());
        }
    }
    else if (command == "spawnallyto")
    {
        MobID::T id;
        while (iss >> arg)
        {
            try { id = std::stoi(arg); }
            catch (const std::invalid_argument&) { continue; }
            catch (const std::out_of_range&) { continue; }
            if (id >= MobID::kNumMobs) continue;
            alloc_mob(simulation, id, x, y, player.get_team());
        }
    }
    */
    else if (command == "killallmobs")
    {
        for (uint16_t i = 0; i < ENTITY_CAP; ++i)
        {
            EntityID id(i, 0);
            // if (!simulation->ent_alive(id)) continue;
            Entity& ent = simulation->get_ent(id);
            if (ent.has_component(kMob))
            {
                ent.health = 0;
            }
        }
    }
    else if (command == "broadcast")
    {
        std::string text;
        std::getline(iss, text);
        if (!text.empty())
        {
            Server::game.broadcast_message(text);
        }
    }
    else if (command == "god")
    {
        if (!player.immunity_ticks)
        {
            player.immunity_ticks = 99999 * TPS;
        }
        else
        {
            player.immunity_ticks = 0;
        }
    }
    else if (command == "heal")
    {
        player.health = player.max_health;
    }
}


void Client::on_disconnect(WebSocket* ws, int code, std::string_view message)
{
    std::printf("disconnect: [%d]\n", code);
    Client* client = ws->getUserData();
    if (client == nullptr) return;
    client->remove();
}

bool Client::check_invalid(bool valid)
{
    if (valid) return false;
    std::cout << "client sent an invalid packet\n";
    //optional
    disconnect();

    return true;
}
