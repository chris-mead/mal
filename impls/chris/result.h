#include <optional>

template<typename T>
class Result
{
    std::optional<T> result;
    std::string error_message;
    // Shut the compiler up
    Token token {TokenKind::SYM, "", 0};

public:
    Result(std::string error_message_, Token token_) :
        error_message {error_message_},
        token {token_}
    {
    }

    Result(T node_) :
        result {node_}
    {
    }

    const std::string& message() const
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
