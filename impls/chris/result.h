#ifndef RESULT_H
#define RESULT_H

#include <optional>
#include <string>

template<typename T>
class Result
{
    std::optional<T> result;
    std::string error_message;
    std::optional<Token> token;

public:
    Result(std::string error_message_, std::optional<Token> token_ = {}) :
        error_message{error_message_},
        token{token_}
    {
    }

    Result(T node_) :
        result{node_}
    {
    }

    const auto& message() const
    {
        return error_message;
    }

    bool error() const
    {
        return !bool(result);
    }

    const T& get() const
    {
        return result.value();
    }

    T& get()
    {
        return result.value();
    }
};

#endif /* RESSULT_H */
