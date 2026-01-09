#pragma once

#include <algorithm>
#include <vector>


// 把 vector 中满足 pred 的元素以“无序删除”方式移除（覆盖 -> pop_back）
template <typename T, typename Alloc, typename Pred>
void unordered_erase_if(std::vector<T, Alloc>& v, Pred pred) {
    std::size_t i = 0;
    while (i < v.size()) {
        if (pred(v[i])) {
            // 如果不是最后一个元素，就把最后一个元素 swap 到当前位置，然后 pop_back
            if (i + 1 != v.size()) {
                if constexpr (std::is_nothrow_move_assignable_v<T>) {
                    v[i] = std::move(v.back());
                } else {
                    std::iter_swap(v.begin() + i, v.end() - 1);
                }
                v.pop_back();
                // 这里不要 ++i，因为被 swap 进来的元素还需重新检查
            } else {
                // 当前就是最后一个元素，直接 pop_back 并结束
                v.pop_back();
                break;
            }
        } else {
            ++i;
        }
    }
}
