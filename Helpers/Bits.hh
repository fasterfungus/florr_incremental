#pragma once

#include <cstdint>

namespace BitMath {
    constexpr uint32_t count(uint32_t v) {return 32 - __builtin_clz(v - 1); }; //计算存储一个数所需的最小位数
    constexpr uint32_t fill(uint32_t v) { return (1 << v) - 1; };  //生成一个数，二进制表示中前v位为1，后面为0
    template<typename T>
    constexpr T at(T const &val, uint32_t bit) { return (val >> bit) & 1; }; // 检查val的第bit位是否为1
    template<typename T>
    constexpr void set(T &val, uint32_t bit) { val |= ((T) 1 << bit); }; //将val的第bit位设置为1
    template<typename T>
    constexpr void unset(T &val, uint32_t bit) { val &= ~((T) 1 << bit); }; //将val的第bit位设置为0

    constexpr uint8_t at_arr(uint8_t const *arr, uint32_t bit) { //检查字节数组中第bit位是否为1
        return at(arr[bit >> 3], bit & 7);
    };
    constexpr void set_arr(uint8_t *arr, uint32_t bit) { //将字节数组中第bit位设置为1
        set(arr[bit >> 3], bit & 7);
    };
    constexpr void unset_arr(uint8_t *arr, uint32_t bit) { //将字节数组中第bit位设置为0
        unset(arr[bit >> 3], bit & 7);
    };
}


class PersistentFlag { //数据持久化
    uint8_t value : 1; //占用一个bit
    uint8_t preserved : 1; //占用一个bit
public:
    PersistentFlag() : value(0), preserved(0) {};
    void set(uint8_t v) { value = v; preserved |= v; } //更新数据状态
    operator uint8_t() const { return preserved; }; // 隐式类型转换
    void clear() { preserved = value; };  //重置preserved为当前value的状态
};
