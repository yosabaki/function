//
// Created by artem on 26.01.19.
//

#ifndef UNTITLED9_FUNCTION_H
#define UNTITLED9_FUNCTION_H

#include <cstddef>
#include <memory>
#include <variant>
#include <array>
#include <cstring>

namespace myns {
    template<typename>
    class function;

    template<typename Ret, typename ... Args>
    class function<Ret(Args...)> {
    private:
        static unsigned const int BUFFER_SIZE = 32;
        struct callable;
        using SmallT = std::array<std::byte, BUFFER_SIZE>;
        using BigT = std::unique_ptr<callable>;
    public:
        function() noexcept : holder(nullptr) {

        }

        function(std::nullptr_t) noexcept : holder(nullptr) {

        }

        explicit operator bool() {
            return is_small() || std::get<BigT>(holder) != nullptr;
        }

        function(const function &other) {
            if (other.is_small()) {
                other.get_small()->small_copy(std::get<SmallT>(holder).data());
            } else {
                holder = std::get<BigT>(other.holder)->big_copy();
            }
        }

        function(function &&other) noexcept {
            if (other.is_small()) {
                other.get_small()->move(std::get<SmallT>(holder).data());
                other.holder = nullptr;
            } else {
                holder = nullptr;
                holder.swap(other.holder);
            }
        }

        template<class FuncT>
        function(FuncT function) {
            if (sizeof(callableFunctor < FuncT > ) <= BUFFER_SIZE) {
                new(std::get<SmallT>(holder).data()) callableFunctor<FuncT>(std::move(function));
            } else {
                holder = std::make_unique<callableFunctor < FuncT>>
                (std::move(function));
            }
        }

        function &operator=(const function &other) {
            function tmp(other);
            swap(tmp);
            return *this;
        }

        ~function() {
            if (is_small()) {
                get_small()->~callable();
            }
        }

        function &operator=(function &&other) noexcept {
            if (is_small()) {
                if (other.is_small()) {
                    get_small()->~callable();
                    other.get_small()->move(std::get<SmallT>(holder).data());
                    other.holder = nullptr;
                } else {
                    get_small()->~callable();
                    std::get<BigT>(other.holder)->move(std::get<SmallT>(holder).data());
                    other.holder = nullptr;
                }
            } else {
                if (other.is_small()) {
                    std::get<BigT>(holder).release();
                    holder = SmallT();
                    other.get_small()->move(std::get<SmallT>(holder).data());
                    other.holder = nullptr;
                } else {
                    holder.swap(other.holder);
                }
            }
            return *this;
        }

        void swap(function &other) noexcept {
            function tmp(std::move(other));
            other = std::move(*this);
            *this = std::move(tmp);
        };

        Ret operator()(Args &&... args) const {
            if (!is_small()) {
                return std::get<BigT>(holder)->call(std::forward<Args>(args) ...);
            } else {
                get_small()->call(std::forward<Args>(args)...);
            }
        };

    private:

        callable *get_small() const {
            return reinterpret_cast<callable *>(std::get<SmallT>(holder).data());
        }

        bool is_small() const {
            return !std::holds_alternative<std::unique_ptr<callable>>(holder);
        }

        struct callable {
            callable() = default;

            virtual ~callable() = default;

            virtual Ret call(Args &&...args) = 0;

            virtual std::unique_ptr<callable> big_copy() const = 0;

            virtual void small_copy(std::byte *buffer) const = 0;

            virtual void move(std::byte *buffer) = 0;
        };

        template<typename FuncT>
        struct callableFunctor : public callable {

            callableFunctor(FuncT &&function) : currFunction(std::move(function)) {}

            callableFunctor(FuncT const &function) : currFunction(function) {}

            virtual void move(std::byte *buffer) {
                new(buffer) callableFunctor<FuncT>(std::move(*this));
            }

            virtual Ret call(Args &&...args) {
                return currFunction(std::forward<Args>(args)...);
            }


            virtual ~callableFunctor() {}

            std::unique_ptr<callable> big_copy() const override {
                return std::make_unique<callableFunctor<FuncT>>(currFunction);
            }

            void small_copy(std::byte *buffer) const override {
                new(buffer)callableFunctor<FuncT>(currFunction);
            }

        private:
            FuncT currFunction;
        };

        mutable std::variant<SmallT, BigT> holder;
    };

}
#endif //UNTITLED9_FUNCTION_H
