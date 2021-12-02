#pragma once
#include <algorithm>
#include <rapidfuzz/details/types.hpp>
#include <string>

namespace rapidfuzz {

template <typename CharT>
class SplittedSentenceView {
public:
    SplittedSentenceView(string_view_vec<CharT> sentence) : m_sentence(std::move(sentence))
    {}

    std::size_t dedupe();
    std::size_t size() const;

    std::size_t length() const
    {
        return size();
    }

    bool empty() const
    {
        return m_sentence.empty();
    }

    std::size_t word_count() const
    {
        return m_sentence.size();
    }

    std::basic_string<CharT> join() const;

    string_view_vec<CharT> words() const
    {
        return m_sentence;
    }

private:
    string_view_vec<CharT> m_sentence;
};

template <typename CharT>
std::size_t SplittedSentenceView<CharT>::dedupe()
{
    std::size_t old_word_count = word_count();
    m_sentence.erase(std::unique(m_sentence.begin(), m_sentence.end()), m_sentence.end());
    return old_word_count - word_count();
}

template <typename CharT>
std::size_t SplittedSentenceView<CharT>::size() const
{
    if (m_sentence.empty()) return 0;

    // there is a whitespace between each word
    std::size_t result = m_sentence.size() - 1;
    for (const auto& word : m_sentence) {
        result += word.size();
    }

    return result;
}

template <typename CharT>
std::basic_string<CharT> SplittedSentenceView<CharT>::join() const
{
    if (m_sentence.empty()) {
        return std::basic_string<CharT>();
    }

    auto sentence_iter = m_sentence.begin();
    std::basic_string<CharT> joined{*sentence_iter};
    const std::basic_string<CharT> whitespace{0x20};
    ++sentence_iter;
    for (; sentence_iter != m_sentence.end(); ++sentence_iter) {
        joined.append(whitespace).append(std::basic_string<CharT>{*sentence_iter});
    }
    return joined;
}

} // namespace rapidfuzz