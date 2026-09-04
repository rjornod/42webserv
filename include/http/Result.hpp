#pragma once
#include <variant>
#include <stdexcept>
#include <utility>

template <typename T, typename E>
class Result {

  private:
    Result(std::in_place_index_t<0>, T value) : m_data(std::in_place_index<0>, std::move(value)) {}
    Result(std::in_place_index_t<1>, E error) : m_data(std::in_place_index<1>, std::move(error)) {}
    std::variant<T, E> m_data;
  
  public:

    //Construct a success
    static Result Ok(T value) {
      return Result(std::in_place_index<0>, std::move(value));
    }

    //Construct a failure
    static Result Err(E error) {
      return Result(std::in_place_index<1>, std::move(error));
    }

    bool isOk() const {return m_data.index() == 0;}
    bool isErr() const {return m_data.index() == 1;}

    explicit operator bool() const {return isOk();}

    //Access the success value (throws if it's actually an error)
    const T& value() const {
      if (!isOk()) 
        throw std::logic_error("Result::value() called on an Err");
      return std::get<0>(m_data);
    }

    //Access the error (throws if it's actually an success)
    const E& error() const {
      if (!isErr()) 
        throw std::logic_error("Result::error() called on an Ok");
      return std::get<1>(m_data);
    }

    // Convenience operators, so you can write *result like std::optional
    const T& operator*() const { return value(); }
    T& operator*() { return value(); }

};