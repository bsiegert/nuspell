/* Copyright 2018-2019 Dimitrij Mijoski, Sander van Geloven
 *
 * This file is part of Nuspell.
 *
 * Nuspell is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nuspell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Nuspell.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief Data structures, private header.
 */

#ifndef NUSPELL_STRUCTURES_HXX
#define NUSPELL_STRUCTURES_HXX

#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <boost/container/small_vector.hpp>
#include <boost/range/iterator_range_core.hpp>

namespace nuspell {
inline namespace v3 {
#define NUSPELL_LITERAL(T, x) ::nuspell::literal_choose<T>(x, L##x)

template <class CharT>
auto constexpr literal_choose(const char* narrow, const wchar_t* wide);
template <>
auto constexpr literal_choose<char>(const char* narrow, const wchar_t*)
{
	return narrow;
}
template <>
auto constexpr literal_choose<wchar_t>(const char*, const wchar_t* wide)
{
	return wide;
}

/**
 * @brief A Set class backed by a string. Very useful for small sets.
 *
 * Has the same interface as std::set.
 */
template <class CharT>
class String_Set {
      private:
	std::basic_string<CharT> d;
	auto sort_uniq() -> void
	{
		auto first = begin();
		auto last = end();
		using t = traits_type;
		sort(first, last, t::lt);
		d.erase(unique(first, last, t::eq), last);
	}
	struct Char_Traits_Less_Than {
		auto operator()(CharT a, CharT b) const noexcept
		{
			return traits_type::lt(a, b);
		}
	};

      public:
	using Str = std::basic_string<CharT>;
	using traits_type = typename Str::traits_type;

	using key_type = typename Str::value_type;
	using key_compare = Char_Traits_Less_Than;
	using value_type = typename Str::value_type;
	using value_compare = key_compare;
	using allocator_type = typename Str::allocator_type;
	using pointer = typename Str::pointer;
	using const_pointer = typename Str::const_pointer;
	using reference = typename Str::reference;
	using const_reference = typename Str::const_reference;
	using size_type = typename Str::size_type;
	using difference_type = typename Str::difference_type;
	using iterator = typename Str::iterator;
	using const_iterator = typename Str::const_iterator;
	using reverse_iterator = typename Str::reverse_iterator;
	using const_reverse_iterator = typename Str::const_reverse_iterator;

	String_Set() = default;
	String_Set(const Str& s) : d(s) { sort_uniq(); }
	String_Set(Str&& s) : d(move(s)) { sort_uniq(); }
	String_Set(const CharT* s) : d(s) { sort_uniq(); }
	template <class InputIterator>
	String_Set(InputIterator first, InputIterator last) : d(first, last)
	{
		sort_uniq();
	}
	String_Set(std::initializer_list<value_type> il) : d(il)
	{
		sort_uniq();
	}

	auto& operator=(const Str& s)
	{
		d = s;
		sort_uniq();
		return *this;
	}
	auto& operator=(Str&& s)
	{
		d = move(s);
		sort_uniq();
		return *this;
	}
	auto& operator=(std::initializer_list<value_type> il)
	{
		d = il;
		sort_uniq();
		return *this;
	}
	auto& operator=(const CharT* s)
	{
		d = s;
		sort_uniq();
		return *this;
	}

	// non standard underlying storage access:
	auto& data() const noexcept { return d; }
	operator const Str&() const noexcept { return d; }

	// iterators:
	iterator begin() noexcept { return d.begin(); }
	const_iterator begin() const noexcept { return d.begin(); }
	iterator end() noexcept { return d.end(); }
	const_iterator end() const noexcept { return d.end(); }

	reverse_iterator rbegin() noexcept { return d.rbegin(); }
	const_reverse_iterator rbegin() const noexcept { return d.rbegin(); }
	reverse_iterator rend() noexcept { return d.rend(); }
	const_reverse_iterator rend() const noexcept { return d.rend(); }

	const_iterator cbegin() const noexcept { return d.cbegin(); }
	const_iterator cend() const noexcept { return d.cend(); }
	const_reverse_iterator crbegin() const noexcept { return d.crbegin(); }
	const_reverse_iterator crend() const noexcept { return d.crend(); }

	// capacity:
	bool empty() const noexcept { return d.empty(); }
	size_type size() const noexcept { return d.size(); }
	size_type max_size() const noexcept { return d.max_size(); }

	// modifiers:
	std::pair<iterator, bool> insert(value_type x)
	{
		auto it = lower_bound(x);
		if (it != end() && *it == x) {
			return {it, false};
		}
		auto ret = d.insert(it, x);
		return {ret, true};
	}
	iterator insert(iterator hint, value_type x)
	{
		if (hint == end() || traits_type::lt(x, *hint)) {
			if (hint == begin() ||
			    traits_type::lt(*(hint - 1), x)) {
				return d.insert(hint, x);
			}
		}
		return insert(x).first;
	}

	// iterator insert(const_iterator hint, value_type&& x);
	template <class InputIterator>
	void insert(InputIterator first, InputIterator last)
	{
		d.insert(end(), first, last);
		sort_uniq();
	}
	void insert(std::initializer_list<value_type> il)
	{
		d.insert(end(), il);
		sort_uniq();
	}
	template <class... Args>
	std::pair<iterator, bool> emplace(Args&&... args)
	{
		return insert(CharT(args...));
	}

	template <class... Args>
	iterator emplace_hint(iterator hint, Args&&... args)
	{
		return insert(hint, CharT(args...));
	}

	iterator erase(iterator position) { return d.erase(position); }
	size_type erase(key_type x)
	{
		auto i = d.find(x);
		if (i != d.npos) {
			d.erase(i, 1);
			return true;
		}
		return false;
	}
	iterator erase(iterator first, iterator last)
	{
		return d.erase(first, last);
	}
	void swap(String_Set& s) { d.swap(s.d); }
	void clear() noexcept { d.clear(); }

	// non standrd modifiers:
	auto insert(const Str& s) -> void
	{
		d += s;
		sort_uniq();
	}
	auto& operator+=(const Str& s)
	{
		insert(s);
		return *this;
	}

	// observers:
	key_compare key_comp() const { return Char_Traits_Less_Than(); }
	value_compare value_comp() const { return key_comp(); }

	// set operations:
      private:
	auto lookup(key_type x) const
	{
		auto i = d.find(x);
		if (i == d.npos)
			i = d.size();
		return i;
	}

      public:
	iterator find(key_type x) { return begin() + lookup(x); }
	const_iterator find(key_type x) const { return begin() + lookup(x); }
	size_type count(key_type x) const { return d.find(x) != d.npos; }

	iterator lower_bound(key_type x)
	{
		return std::lower_bound(begin(), end(), x, traits_type::lt);
	}

	const_iterator lower_bound(key_type x) const
	{
		return std::lower_bound(begin(), end(), x, traits_type::lt);
	}
	iterator upper_bound(key_type x)
	{
		return std::upper_bound(begin(), end(), x, traits_type::lt);
	}

	const_iterator upper_bound(key_type x) const
	{
		return std::upper_bound(begin(), end(), x, traits_type::lt);
	}
	std::pair<iterator, iterator> equal_range(key_type x)
	{
		return std::equal_range(begin(), end(), x, traits_type::lt);
	}

	std::pair<const_iterator, const_iterator> equal_range(key_type x) const
	{
		return std::equal_range(begin(), end(), x, traits_type::lt);
	}

	// non standard set operations:
	bool contains(key_type x) const { return count(x); }

	// compare
	bool operator<(const String_Set& rhs) const { return d < rhs.d; }
	bool operator<=(const String_Set& rhs) const { return d <= rhs.d; }
	bool operator==(const String_Set& rhs) const { return d == rhs.d; }
	bool operator!=(const String_Set& rhs) const { return d != rhs.d; }
	bool operator>=(const String_Set& rhs) const { return d >= rhs.d; }
	bool operator>(const String_Set& rhs) const { return d > rhs.d; }
};

template <class CharT>
auto swap(String_Set<CharT>& a, String_Set<CharT>& b)
{
	a.swap(b);
}

using Flag_Set = String_Set<char16_t>;

template <class CharT>
class Substr_Replacer {
      public:
	using Str = std::basic_string<CharT>;
	using Str_View = std::basic_string_view<CharT>;
	using Pair_Str = std::pair<Str, Str>;
	using Table_Pairs = std::vector<Pair_Str>;

      private:
	Table_Pairs table;
	auto sort_uniq() -> void;
	auto find_match(Str_View s) const;

      public:
	Substr_Replacer() = default;
	Substr_Replacer(const Table_Pairs& v) : table(v) { sort_uniq(); }
	Substr_Replacer(Table_Pairs&& v) : table(move(v)) { sort_uniq(); }

	auto& operator=(const Table_Pairs& v)
	{
		table = v;
		sort_uniq();
		return *this;
	}
	auto& operator=(Table_Pairs&& v)
	{
		table = move(v);
		sort_uniq();
		return *this;
	}

	auto replace(Str& s) const -> Str&;
	auto replace_copy(Str s) const -> Str
	{
		replace(s);
		return s;
	}
};
template <class CharT>
auto Substr_Replacer<CharT>::sort_uniq() -> void
{
	auto first = begin(table);
	auto last = end(table);
	sort(first, last, [](auto& a, auto& b) { return a.first < b.first; });
	auto it = unique(first, last,
	                 [](auto& a, auto& b) { return a.first == b.first; });
	table.erase(it, last);

	// remove empty key ""
	if (!table.empty() && table.front().first.empty())
		table.erase(begin(table));
}

template <class CharT>
auto Substr_Replacer<CharT>::find_match(Str_View s) const
{
	auto& t = table;
	struct Comparer_Str_Rep {
		auto static cmp_prefix_of(const Str& p, Str_View of)
		{
			return p.compare(0, p.npos, of.data(),
			                 std::min(p.size(), of.size()));
		}
		auto operator()(const Pair_Str& a, Str_View b)
		{
			return cmp_prefix_of(a.first, b) < 0;
		}
		auto operator()(Str_View a, const Pair_Str& b)
		{
			return cmp_prefix_of(b.first, a) > 0;
		}
		auto static eq(const Pair_Str& a, Str_View b)
		{
			return cmp_prefix_of(a.first, b) == 0;
		}
	};
	Comparer_Str_Rep csr;
	auto it = begin(t);
	auto last_match = end(t);
	for (;;) {
		auto it2 = upper_bound(it, end(t), s, csr);
		if (it2 == it) {
			// not found, s is smaller that the range
			break;
		}
		--it2;
		if (csr.eq(*it2, s)) {
			// Match found. Try another search maybe for
			// longer.
			last_match = it2;
			it = ++it2;
		}
		else {
			// not found, s is greater that the range
			break;
		}
	}
	return last_match;
}

template <class CharT>
auto Substr_Replacer<CharT>::replace(Str& s) const -> Str&
{

	if (table.empty())
		return s;
	for (size_t i = 0; i < s.size(); /*no increment here*/) {
		auto substr = Str_View(&s[i], s.size() - i);
		auto it = find_match(substr);
		if (it != end(table)) {
			auto& match = *it;
			// match found. match.first is the found string,
			// match.second is the replacement.
			s.replace(i, match.first.size(), match.second);
			i += match.second.size();
			continue;
		}
		++i;
	}
	return s;
}

/**
 * @brief Casing type enum, ignoring neutral case characters.
 */
enum class Casing : char {
	SMALL /**< all lower case or neutral case, e.g. "lowercase" or "123" */,
	INIT_CAPITAL /**< start upper case, rest lower case, e.g. "Initcap" */,
	ALL_CAPITAL /**< all upper case, e.g. "UPPERCASE" or "ALL4ONE" */,
	CAMEL /**< camel case, start lower case, e.g. "camelCase" */,
	PASCAL /**< pascal case, start upper case, e.g. "PascalCase" */
};

template <class CharT>
class Break_Table {
      public:
	using Str = std::basic_string<CharT>;
	using Table_Str = std::vector<Str>;
	using iterator = typename Table_Str::iterator;
	using const_iterator = typename Table_Str::const_iterator;

      private:
	Table_Str table;
	size_t start_word_breaks_last_idx = 0;
	size_t end_word_breaks_last_idx = 0;

	auto order_entries() -> void;

      public:
	Break_Table() = default;
	Break_Table(const Table_Str& v) : table(v) { order_entries(); }
	Break_Table(Table_Str&& v) : table(move(v)) { order_entries(); }

	auto& operator=(const Table_Str& v)
	{
		table = v;
		order_entries();
		return *this;
	}

	auto& operator=(Table_Str&& v)
	{
		table = move(v);
		order_entries();
		return *this;
	}

	auto start_word_breaks() const -> boost::iterator_range<const_iterator>
	{
		return {begin(table),
		        begin(table) + start_word_breaks_last_idx};
	}
	auto end_word_breaks() const -> boost::iterator_range<const_iterator>
	{
		return {begin(table) + start_word_breaks_last_idx,
		        begin(table) + end_word_breaks_last_idx};
	}
	auto middle_word_breaks() const -> boost::iterator_range<const_iterator>
	{
		return {begin(table) + end_word_breaks_last_idx, end(table)};
	}
};
template <class CharT>
auto Break_Table<CharT>::order_entries() -> void
{
	auto it = remove_if(begin(table), end(table), [](auto& s) {
		return s.empty() ||
		       (s.size() == 1 && (s[0] == '^' || s[0] == '$'));
	});
	table.erase(it, end(table));

	auto is_start_word_break = [=](auto& x) { return x[0] == '^'; };
	auto is_end_word_break = [=](auto& x) { return x.back() == '$'; };
	auto start_word_breaks_last =
	    partition(begin(table), end(table), is_start_word_break);
	start_word_breaks_last_idx = start_word_breaks_last - begin(table);

	for_each(begin(table), start_word_breaks_last,
	         [](auto& e) { e.erase(0, 1); });

	auto end_word_breaks_last =
	    partition(start_word_breaks_last, end(table), is_end_word_break);
	end_word_breaks_last_idx = end_word_breaks_last - begin(table);

	for_each(start_word_breaks_last, end_word_breaks_last,
	         [](auto& e) { e.pop_back(); });
}

struct identity {
	template <class T>
	constexpr auto operator()(T&& t) const noexcept -> T&&
	{
		return std::forward<T>(t);
	}
};

template <class Value, class Key = Value, class KeyExtract = identity>
class Hash_Multiset {
      private:
	using bucket_type = boost::container::small_vector<Value, 1>;
	static constexpr float max_load_fact = 7.0 / 8.0;
	std::vector<bucket_type> data;
	size_t sz = 0;
	size_t max_load_factor_capacity = 0;

      public:
	using key_type = Key;
	using value_type = Value;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using hasher = std::hash<Key>;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = typename bucket_type::pointer;
	using const_pointer = typename bucket_type::const_pointer;
	using local_iterator = typename bucket_type::iterator;
	using local_const_iterator = typename bucket_type::const_iterator;

	Hash_Multiset() = default;

	auto size() const { return sz; }
	auto empty() const { return size() == 0; }

	auto rehash(size_t count)
	{
		if (empty()) {
			size_t capacity = 16;
			while (capacity <= count)
				capacity <<= 1;
			data.resize(capacity);
			max_load_factor_capacity =
			    std::ceil(capacity * max_load_fact);
			return;
		}
		if (count < size() / max_load_fact)
			count = size() / max_load_fact;
		auto n = Hash_Multiset();
		n.rehash(count);
		for (auto& b : data) {
			for (auto& x : b) {
				n.insert(x);
			}
		}
		data.swap(n.data);
		sz = n.sz;
		max_load_factor_capacity = n.max_load_factor_capacity;
	}

	auto reserve(size_t count) -> void
	{
		rehash(std::ceil(count / max_load_fact));
	}

	auto insert(const_reference value)
	{
		using namespace std;
		auto hash = hasher();
		auto key_extract = KeyExtract();
		if (sz == max_load_factor_capacity) {
			reserve(sz + 1);
		}
		auto&& key = key_extract(value);
		auto h = hash(key);
		auto h_mod = h & (data.size() - 1);
		auto& bucket = data[h_mod];
		if (bucket.size() == 0 || bucket.size() == 1 ||
		    key == key_extract(bucket.back())) {
			bucket.push_back(value);
			++sz;
			return end(bucket) - 1;
		}
		auto last =
		    std::find_if(rbegin(bucket), rend(bucket), [&](auto& x) {
			    return key == key_extract(x);
		    });
		if (last != rend(bucket)) {
			auto ret = bucket.insert(last.base(), value);
			++sz;
			return ret;
		}

		bucket.push_back(value);
		++sz;
		return end(bucket) - 1;
	}
	template <class... Args>
	auto emplace(Args&&... a)
	{
		return insert(value_type(std::forward<Args>(a)...));
	}

	auto equal_range(const key_type& key) const
	    -> std::pair<local_const_iterator, local_const_iterator>
	{
		using namespace std;
		auto hash = hasher();
		auto key_extract = KeyExtract();
		if (data.empty())
			return {};
		auto h = hash(key);
		auto h_mod = h & (data.size() - 1);
		auto& bucket = data[h_mod];
		if (bucket.empty())
			return {};
		if (bucket.size() == 1) {
			if (key == key_extract(bucket.front()))
				return {begin(bucket), end(bucket)};
			return {};
		}
		auto first =
		    std::find_if(begin(bucket), end(bucket), [&](auto& x) {
			    return key == key_extract(x);
		    });
		if (first == end(bucket))
			return {};
		auto next = first + 1;
		if (next == end(bucket) || key != key_extract(*next))
			return {first, next};
		auto last =
		    std::find_if(rbegin(bucket), rend(bucket), [&](auto& x) {
			    return key == key_extract(x);
		    });
		return {first, last.base()};
	}
};

struct Condition_Exception : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

/**
 * @brief Limited regular expression matching used in affix entries.
 *
 * This implementation increases performance over the regex implementation in
 * the standard library.
 */
template <class CharT>
class Condition {
	using Str = std::basic_string<CharT>;
	enum Span_Type {
		NORMAL /**< normal character */,
		DOT /**< wildcard character */,
		ANY_OF /**< set of possible characters */,
		NONE_OF /**< set of excluding characters */
	};
	struct Span {
		size_t pos = {};
		size_t len = {};
		Span_Type type = {};
		Span() = default;
		Span(size_t pos, size_t len, Span_Type type)
		    : pos(pos), len(len), type(type)
		{
		}
	};

	Str cond;
	std::vector<Span> spans; // pos, len, type
	size_t length = 0;

	auto construct() -> void;

      public:
	Condition() = default;
	Condition(const Str& condition) : cond(condition) { construct(); }
	Condition(Str&& condition) : cond(move(condition)) { construct(); }
	Condition(const CharT* condition) : cond(condition) { construct(); }
	auto& operator=(const Str& condition)
	{
		cond = condition;
		length = 0;
		construct();
		return *this;
	}
	auto& operator=(Str&& condition)
	{
		cond = std::move(condition);
		length = 0;
		construct();
		return *this;
	}
	auto& operator=(const CharT* condition)
	{
		cond = condition;
		length = 0;
		construct();
		return *this;
	}
	auto match(const Str& s, size_t pos = 0, size_t len = Str::npos) const
	    -> bool;
	auto match_prefix(const Str& s) const { return match(s, 0, length); }
	auto match_suffix(const Str& s) const
	{
		if (length > s.size())
			return false;
		return match(s, s.size() - length, length);
	}
};
template <class CharT>
auto Condition<CharT>::construct() -> void
{
	size_t i = 0;
	for (; i != cond.size();) {
		size_t j = cond.find_first_of(NUSPELL_LITERAL(CharT, "[]."), i);
		if (i != j) {
			if (j == cond.npos) {
				spans.emplace_back(i, cond.size() - i, NORMAL);
				length += cond.size() - i;
				break;
			}
			spans.emplace_back(i, j - i, NORMAL);
			length += j - i;
			i = j;
		}
		if (cond[i] == '.') {
			spans.emplace_back(i, 1, DOT);
			++length;
			++i;
			continue;
		}
		if (cond[i] == ']') {
			auto what =
			    "closing bracket has no matching opening bracket";
			throw Condition_Exception(what);
		}
		if (cond[i] == '[') {
			++i;
			if (i == cond.size()) {
				auto what = "opening bracket has no matching "
				            "closing bracket";
				throw Condition_Exception(what);
			}
			Span_Type type;
			if (cond[i] == '^') {
				type = NONE_OF;
				++i;
			}
			else {
				type = ANY_OF;
			}
			j = cond.find(']', i);
			if (j == i) {
				auto what = "empty bracket expression";
				throw Condition_Exception(what);
			}
			if (j == cond.npos) {
				auto what = "opening bracket has no matching "
				            "closing bracket";
				throw Condition_Exception(what);
			}
			spans.emplace_back(i, j - i, type);
			++length;
			i = j + 1;
		}
	}
}

/**
 * Checks if provided string matched the condition.
 *
 * @param s string to check if it matches the condition.
 * @param pos start position for string, default is 0.
 * @param len length of string counting from the start position.
 * @return The valueof true when string matched condition.
 */
template <class CharT>
auto Condition<CharT>::match(const Str& s, size_t pos, size_t len) const -> bool
{
	if (pos > s.size()) {
		throw std::out_of_range(
		    "position on the string is out of bounds");
	}
	if (s.size() - pos < len)
		len = s.size() - pos;
	if (len != length)
		return false;

	size_t i = pos;
	for (auto& x : spans) {
		using tr = typename Str::traits_type;
		switch (x.type) {
		case NORMAL:
			if (tr::compare(&s[i], &cond[x.pos], x.len) == 0)
				i += x.len;
			else
				return false;
			break;
		case DOT:
			++i;
			break;
		case ANY_OF:
			if (tr::find(&cond[x.pos], x.len, s[i]))
				++i;
			else
				return false;
			break;
		case NONE_OF:
			if (tr::find(&cond[x.pos], x.len, s[i]))
				return false;
			else
				++i;
			break;
		}
	}
	return true;
}

template <class CharT>
class Prefix {
      public:
	using Str = std::basic_string<CharT>;
	using Cond = Condition<CharT>;
	using value_type = CharT;

	char16_t flag = 0;
	bool cross_product = false;
	Str stripping;
	Str appending;
	Flag_Set cont_flags;
	Cond condition;

	auto to_root(Str& word) const -> Str&
	{
		return word.replace(0, appending.size(), stripping);
	}
	auto to_root_copy(Str word) const -> Str
	{
		to_root(word);
		return word;
	}

	auto to_derived(Str& word) const -> Str&
	{
		return word.replace(0, stripping.size(), appending);
	}
	auto to_derived_copy(Str word) const -> Str
	{
		to_derived(word);
		return word;
	}

	auto check_condition(const Str& word) const -> bool
	{
		return condition.match_prefix(word);
	}
};

template <class CharT>
class Suffix {
      public:
	using Str = std::basic_string<CharT>;
	using Cond = Condition<CharT>;
	using value_type = CharT;

	char16_t flag = 0;
	bool cross_product = false;
	Str stripping;
	Str appending;
	Flag_Set cont_flags;
	Cond condition;

	auto to_root(Str& word) const -> Str&
	{
		return word.replace(word.size() - appending.size(),
		                    appending.size(), stripping);
	}
	auto to_root_copy(Str word) const -> Str
	{
		to_root(word);
		return word;
	}

	auto to_derived(Str& word) const -> Str&
	{
		return word.replace(word.size() - stripping.size(),
		                    stripping.size(), appending);
	}
	auto to_derived_copy(Str word) const -> Str
	{
		to_derived(word);
		return word;
	}

	auto check_condition(const Str& word) const -> bool
	{
		return condition.match_suffix(word);
	}
};

template <class T, class Key_Extr = identity, class Key_Transform = identity>
class Prefix_Multiset {
      public:
	using Value_Type = T;
	using Key_Type = std::remove_reference_t<decltype(
	    std::declval<Key_Extr>()(std::declval<T>()))>;
	using Char_Type = typename Key_Type::value_type;
	using Traits = typename Key_Type::traits_type;
	using Vector_Type = std::vector<T>;
	using Iterator = typename Vector_Type::const_iterator;

      private:
	struct Ebo_Key_Extr : public Key_Extr {
	};
	struct Ebo_Key_Transf : public Key_Transform {
	};
	struct Ebo : public Ebo_Key_Extr, Ebo_Key_Transf {
		Vector_Type table;
	} ebo;
	std::basic_string<Char_Type> first_letter;
	std::vector<size_t> prefix_idx_with_first_letter;

	auto key_extractor() const -> const Ebo_Key_Extr& { return ebo; }
	auto key_transformator() const -> const Ebo_Key_Transf& { return ebo; }
	auto& get_table() { return ebo.table; }
	auto& get_table() const { return ebo.table; }

	auto sort()
	{
		auto& extract_key = key_extractor();
		auto& transform_key = key_transformator();
		auto& table = get_table();

		std::sort(begin(table), end(table), [&](T& a, T& b) {
			auto&& key_a = transform_key(extract_key(a));
			auto&& key_b = transform_key(extract_key(b));
			return key_a < key_b;
		});

		first_letter.clear();
		prefix_idx_with_first_letter.clear();
		auto first = begin(table);
		auto last = end(table);
		auto it = std::find_if_not(first, last, [=](const T& x) {
			auto&& k = transform_key(extract_key(x));
			return k.empty();
		});
		while (it != last) {
			auto&& k1 = transform_key(extract_key(*it));
			first_letter.push_back(k1[0]);
			prefix_idx_with_first_letter.push_back(it - first);

			it = std::upper_bound(
			    it, last, k1[0],
			    Comparator{0, extract_key, transform_key});
		}
		if (!prefix_idx_with_first_letter.empty())
			prefix_idx_with_first_letter.push_back(last - first);
	}

	struct Comparator {
		size_t i;
		Key_Extr extract_key;
		Key_Transform transform_key;
		auto operator()(const T& a, Char_Type b) const
		{
			auto&& key_a = transform_key(extract_key(a));
			return Traits::lt(key_a[i], b);
		}
		auto operator()(Char_Type a, const T& b) const
		{
			auto&& key_b = transform_key(extract_key(b));
			return Traits::lt(a, key_b[i]);
		}
	};

      public:
	Prefix_Multiset() = default;
	Prefix_Multiset(Key_Extr ke, Key_Transform kt = {})
	    : ebo{{ke}, {kt}, {}}
	{
	}
	Prefix_Multiset(const Vector_Type& v, Key_Extr ke = {},
	                Key_Transform kt = {})
	    : ebo{{ke}, {kt}, v}
	{
		sort();
	}
	Prefix_Multiset(Vector_Type&& v, Key_Extr ke = {},
	                Key_Transform kt = {})
	    : ebo{{ke}, {kt}, std::move(v)}
	{
		sort();
	}
	Prefix_Multiset(std::initializer_list<T> list, Key_Extr ke = {},
	                Key_Transform kt = {})
	    : ebo{{ke}, {kt}, list}
	{
		sort();
	}
	auto& operator=(const Vector_Type& v)
	{
		get_table() = v;
		sort();
		return *this;
	}
	auto& operator=(Vector_Type&& v)
	{
		get_table() = std::move(v);
		sort();
		return *this;
	}
	auto& operator=(std::initializer_list<T> list)
	{
		get_table() = list;
		sort();
		return *this;
	}
	auto& data() const { return get_table(); }

	template <class Func>
	auto for_each_prefixes_of(const Key_Type& word, Func func) const;

	template <class OutIter>
	auto copy_all_prefixes_of(const Key_Type& word, OutIter o) const
	{
		for_each_prefixes_of(word, [&](const T& x) { *o++ = x; });
		return o;
	}

	class Iter_Prefixes_Of {
		const Prefix_Multiset* set = {};
		Iterator it = {};
		Iterator last = {};
		const Key_Type* search_key = {};
		size_t len = {};
		bool valid = false;

		auto advance() -> void;

	      public:
		using iterator_category = std::input_iterator_tag;
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = const T&;
		using pointer = const T*;

		Iter_Prefixes_Of() = default;
		Iter_Prefixes_Of(const Prefix_Multiset& set,
		                 const Key_Type& word)
		    : set(&set), it(set.get_table().begin()),
		      last(set.get_table().end()), search_key(&word),
		      valid(true)
		{
			advance();
		}
		Iter_Prefixes_Of(const Prefix_Multiset&, Key_Type&&) = delete;
		Iter_Prefixes_Of(Prefix_Multiset&&, const Key_Type&) = delete;
		Iter_Prefixes_Of(Prefix_Multiset&&, Key_Type&&) = delete;

		auto& operator++()
		{
			++it;
			advance();
			return *this;
		}
		auto& operator++(int)
		{
			auto old = *this;
			++*this;
			return old;
		}
		auto& operator*() const { return *it; }
		auto operator-> () const { return &*it; }
		auto operator==(const Iter_Prefixes_Of& other) const
		{
			return valid == other.valid;
		}
		auto operator!=(const Iter_Prefixes_Of& other) const
		{
			return valid != other.valid;
		}

		operator bool() const { return valid; }

		auto begin() const { return *this; }
		auto end() const { return Iter_Prefixes_Of(); }
	};

	auto iterate_prefixes_of(const Key_Type& word) const
	{
		return Iter_Prefixes_Of(*this, word);
	}
	auto iterate_prefixes_of(Key_Type&& word) const = delete;
};
template <class T, class Key_Extr, class Key_Transform>
auto Prefix_Multiset<T, Key_Extr, Key_Transform>::Iter_Prefixes_Of::advance()
    -> void
{
	auto& extract_key = set->key_extractor();
	auto& transform_key = set->key_transformator();
	auto& table = set->get_table();

	if (len == 0) {
		if (it != last) {
			if (transform_key(extract_key(*it)).empty())
				return;
			if (transform_key(*search_key).empty()) {
				valid = false;
				return;
			}

			auto& first_letter = set->first_letter;
			auto& prefix_idx_with_first_letter =
			    set->prefix_idx_with_first_letter;
			auto idx =
			    first_letter.find(transform_key(*search_key)[0]);
			if (idx == first_letter.npos) {
				valid = false;
				return;
			}

			auto first = table.begin();
			it = first + prefix_idx_with_first_letter[idx];
			last = first + prefix_idx_with_first_letter[idx + 1];
			++len;
		}
		else {
			valid = false;
			return;
		}
	}
	for (;; ++len) {
		if (it != last) {
			if (transform_key(extract_key(*it)).size() == len)
				return;
			if (len == transform_key(*search_key).size()) {
				valid = false;
				break;
			}
		}
		else {
			valid = false;
			break;
		}
		tie(it, last) =
		    equal_range(it, last, transform_key(*search_key)[len],
		                Comparator{len, extract_key, transform_key});
	}
}

template <class T, class Key_Extr, class Key_Transform>
template <class Func>
auto Prefix_Multiset<T, Key_Extr, Key_Transform>::for_each_prefixes_of(
    const Key_Type& word, Func func) const
{
	auto& extract_key = key_extractor();
	auto& transform_key = key_transformator();
	auto& table = get_table();

	auto first = begin(table);
	auto last = end(table);
	auto it = first;
	for (; it != last; ++it) {
		auto&& k = transform_key(extract_key(*it));
		if (k.empty())
			func(*it);
		else
			break;
	}

	if (it == last)
		return;
	if (transform_key(word).empty())
		return;

	auto idx = first_letter.find(transform_key(word)[0]);
	if (idx == first_letter.npos)
		return;
	first = begin(table) + prefix_idx_with_first_letter[idx];
	last = begin(table) + prefix_idx_with_first_letter[idx + 1];

	for (size_t len = 1;; ++len) {
		it = first;
		for (; it != last &&
		       transform_key(extract_key(*it)).size() == len;
		     ++it) {
			func(*it);
		}
		if (it == last)
			break;
		if (len == transform_key(word).size())
			break;
		tie(first, last) =
		    equal_range(it, last, transform_key(word)[len],
		                Comparator{len, extract_key, transform_key});
	}
}

template <class CharT>
class Reversed_String_View {
      public:
	using Str = std::basic_string<CharT>;
	using traits_type = typename Str::traits_type;
	using value_type = typename Str::value_type;
	using size_type = typename Str::size_type;
	using const_iterator = typename Str::const_reverse_iterator;

      private:
	const_iterator first = {};
	size_type sz = {};

      public:
	Reversed_String_View() = default;
	Reversed_String_View(const Str& s) : first(s.rbegin()), sz(s.size()) {}
	Reversed_String_View(Str&& s) = delete;
	auto& operator[](size_type i) const { return first[i]; }
	auto size() const { return sz; }
	auto empty() const { return sz == 0; }
	auto begin() const { return first; }
	auto end() const { return first + sz; }
	auto operator<(Reversed_String_View other) const
	{
		return lexicographical_compare(begin(), end(), other.begin(),
		                               other.end(), traits_type::lt);
	}
};

template <class CharT>
struct String_Reverser {
	auto operator()(const std::basic_string<CharT>& x) const
	{
		return Reversed_String_View<CharT>(x);
	}
	// auto operator()(T&& x) const = delete;
};

template <class AffixT>
struct Extractor_Of_Appending_From_Affix {
	auto& operator()(const AffixT& a) const { return a.appending; }
};

template <class T, class Key_Extr = identity>
using Suffix_Multiset = Prefix_Multiset<
    T, Key_Extr,
    String_Reverser<typename Prefix_Multiset<T, Key_Extr>::Char_Type>>;

template <class T, class Key_Extr = identity>
class Affix_Table {
};

class Prefix_Table {
	using Prefix_Multiset_Type =
	    Prefix_Multiset<Prefix<wchar_t>,
	                    Extractor_Of_Appending_From_Affix<Prefix<wchar_t>>>;
	using Key_Type = typename Prefix_Multiset_Type::Key_Type;
	using Vector_Type = typename Prefix_Multiset_Type::Vector_Type;
	Prefix_Multiset_Type table;
	Flag_Set all_cont_flags;

	auto populate()
	{
		for (auto& x : table.data())
			all_cont_flags += x.cont_flags;
	}

      public:
	Prefix_Table() = default;
	Prefix_Table(const Vector_Type& t) : table(t) { populate(); }
	Prefix_Table(Vector_Type&& t) : table(std::move(t)) { populate(); }
	auto& operator=(const Vector_Type& t)
	{
		table = t;
		populate();
		return *this;
	}
	auto& operator=(Vector_Type&& t)
	{
		table = std::move(t);
		populate();
		return *this;
	}
	auto has_continuation_flags() const
	{
		return all_cont_flags.size() != 0;
	}
	auto has_continuation_flag(char16_t flag) const
	{
		return all_cont_flags.contains(flag);
	}
	auto iterate_prefixes_of(const Key_Type& word) const
	{
		return table.iterate_prefixes_of(word);
	}
	auto iterate_prefixes_of(Key_Type&& word) const = delete;
};

class Suffix_Table {
	using Suffix_Multiset_Type =
	    Suffix_Multiset<Suffix<wchar_t>,
	                    Extractor_Of_Appending_From_Affix<Suffix<wchar_t>>>;
	using Key_Type = typename Suffix_Multiset_Type::Key_Type;
	using Vector_Type = typename Suffix_Multiset_Type::Vector_Type;
	Suffix_Multiset_Type table;
	Flag_Set all_cont_flags;

	auto populate()
	{
		for (auto& x : table.data())
			all_cont_flags += x.cont_flags;
	}

      public:
	Suffix_Table() = default;
	Suffix_Table(const Vector_Type& t) : table(t) { populate(); }
	Suffix_Table(Vector_Type&& t) : table(std::move(t)) { populate(); }
	auto& operator=(const Vector_Type& t)
	{
		table = t;
		populate();
		return *this;
	}
	auto& operator=(Vector_Type&& t)
	{
		table = std::move(t);
		populate();
		return *this;
	}
	auto has_continuation_flags() const
	{
		return all_cont_flags.size() != 0;
	}
	auto has_continuation_flag(char16_t flag) const
	{
		return all_cont_flags.contains(flag);
	}
	auto iterate_suffixes_of(const Key_Type& word) const
	{
		return table.iterate_prefixes_of(word);
	}
	auto iterate_suffixes_of(Key_Type&& word) const = delete;
};

template <class CharT>
class String_Pair {
	using Str = std::basic_string<CharT>;
	using Str_View = std::basic_string_view<CharT>;
	size_t i = 0;
	Str s;

      public:
	String_Pair() = default;
	/**
	 * @brief Construct string pair
	 *
	 * Constructs a string pair from a single string containing a pair of
	 * strings and an index where the split resides.
	 *
	 * @param str the string that can be split into a pair.
	 * @param i the index where the string is split.
	 * @throws std::out_of_range
	 */
	template <class Str1>
	String_Pair(Str1&& str, size_t i) : i(i), s(std::forward<Str1>(str))
	{
		if (i > s.size()) {
			throw std::out_of_range("word split is too long");
		}
	}

	template <class Str1, class Str2,
	          class = std::enable_if_t<
	              std::is_same<std::remove_reference_t<Str1>, Str>::value &&
	              std::is_same<std::remove_reference_t<Str2>, Str>::value>>
	String_Pair(Str1&& first, Str2&& second)
	    : i(first.size()) /* must be init before s, before we move
	                         from first */
	      ,
	      s(std::forward<Str1>(first) + std::forward<Str2>(second))

	{
	}
	auto first() const { return Str_View(s).substr(0, i); }
	auto second() const { return Str_View(s).substr(i); }
	auto first(Str_View x)
	{
		s.replace(0, i, x);
		i = x.size();
	}
	auto second(Str_View x) { s.replace(i, s.npos, x); }
	auto& str() const { return s; }
	auto idx() const { return i; }
};
template <class CharT>
struct Compound_Pattern {
	using StrT = std::basic_string<CharT>;

	String_Pair<CharT> begin_end_chars;
	StrT replacement;
	char16_t first_word_flag = 0;
	char16_t second_word_flag = 0;
	bool match_first_only_unaffixed_or_zero_affixed = false;
};

class Compound_Rule_Table {
	std::vector<std::u16string> rules;
	Flag_Set all_flags;

	auto fill_all_flags() -> void;

      public:
	Compound_Rule_Table() = default;
	Compound_Rule_Table(const std::vector<std::u16string>& tbl) : rules(tbl)
	{
		fill_all_flags();
	}
	Compound_Rule_Table(std::vector<std::u16string>&& tbl)
	    : rules(move(tbl))
	{
		fill_all_flags();
	}
	auto& operator=(const std::vector<std::u16string>& tbl)
	{
		rules = tbl;
		fill_all_flags();
		return *this;
	}
	auto& operator=(std::vector<std::u16string>&& tbl)
	{
		rules = move(tbl);
		fill_all_flags();
		return *this;
	}
	auto empty() const { return rules.empty(); }
	auto has_any_of_flags(const Flag_Set& f) const -> bool;
	auto match_any_rule(const std::vector<const Flag_Set*>& data) const
	    -> bool;
};
auto inline Compound_Rule_Table::fill_all_flags() -> void
{
	for (auto& f : rules) {
		all_flags += f;
	}
	all_flags.erase(u'?');
	all_flags.erase(u'*');
}

auto inline Compound_Rule_Table::has_any_of_flags(const Flag_Set& f) const
    -> bool
{
	using std::begin;
	using std::end;
	struct Out_Iter_One_Bool {
		bool* value = nullptr;
		auto& operator++() { return *this; }
		auto& operator++(int) { return *this; }
		auto& operator*() { return *this; }
		auto& operator=(char16_t)
		{
			*value = true;
			return *this;
		}
	};
	auto has_intersection = false;
	auto out_it = Out_Iter_One_Bool{&has_intersection};
	std::set_intersection(begin(all_flags), end(all_flags), begin(f),
	                      end(f), out_it);
	return has_intersection;
}

template <class DataIter, class PatternIter, class FuncEq = std::equal_to<>>
auto match_simple_regex(DataIter data_first, DataIter data_last,
                        PatternIter pat_first, PatternIter pat_last,
                        FuncEq eq = FuncEq())
{
	auto s = std::stack<std::pair<DataIter, PatternIter>>();
	s.emplace(data_first, pat_first);
	auto data_it = DataIter();
	auto pat_it = PatternIter();
	while (!s.empty()) {
		std::tie(data_it, pat_it) = s.top();
		s.pop();
		if (pat_it == pat_last) {
			if (data_it == data_last)
				return true;
			else
				return false;
		}
		auto node_type = *pat_it;
		if (pat_it + 1 == pat_last)
			node_type = 0;
		else
			node_type = *(pat_it + 1);
		switch (node_type) {
		case '?':
			s.emplace(data_it, pat_it + 2);
			if (data_it != data_last && eq(*data_it, *pat_it))
				s.emplace(data_it + 1, pat_it + 2);
			break;
		case '*':
			s.emplace(data_it, pat_it + 2);
			if (data_it != data_last && eq(*data_it, *pat_it))
				s.emplace(data_it + 1, pat_it);

			break;
		default:
			if (data_it != data_last && eq(*data_it, *pat_it))
				s.emplace(data_it + 1, pat_it + 1);
			break;
		}
	}
	return false;
}

template <class DataRange, class PatternRange, class FuncEq = std::equal_to<>>
auto match_simple_regex(const DataRange& data, const PatternRange& pattern,
                        FuncEq eq = FuncEq())
{
	using namespace std;
	return match_simple_regex(begin(data), end(data), begin(pattern),
	                          end(pattern), eq);
}

auto inline match_compund_rule(const std::vector<const Flag_Set*>& words_data,
                               const std::u16string& pattern)
{
	return match_simple_regex(
	    words_data, pattern,
	    [](const Flag_Set* d, char16_t p) { return d->contains(p); });
}

auto inline Compound_Rule_Table::match_any_rule(
    const std::vector<const Flag_Set*>& data) const -> bool
{
	return any_of(begin(rules), end(rules), [&](const std::u16string& p) {
		return match_compund_rule(data, p);
	});
}

template <class CharT>
class Simple_Short_String {
	using value_type = CharT;
	using traits_type = std::char_traits<value_type>;
	using size_type = size_t;
	using Str_View = std::basic_string_view<value_type>;

	static constexpr size_type short_capacity = 22;
	size_type sz = {};
	bool is_long = {};
	union {
		value_type short_data[short_capacity + 1];
		value_type* long_data;
	};

	auto is_short() const -> bool { return !is_long; }

      public:
	Simple_Short_String() { short_data[0] = {}; }
	Simple_Short_String(Str_View other)
	    : sz(other.size()), is_long(sz > short_capacity)
	{
		value_type* ptr;
		if (is_short()) {
			ptr = short_data;
		}
		else {
			long_data = new value_type[sz + 1];
			ptr = long_data;
		}
		traits_type::copy(ptr, other.data(), sz);
		ptr[sz] = {};
	}
	~Simple_Short_String()
	{
		if (is_long)
			delete[] long_data;
	}

	operator Str_View() const noexcept
	{
		if (is_short())
			return {short_data, sz};
		return {long_data, sz};
	}

	Simple_Short_String(const Simple_Short_String& other)
	    : Simple_Short_String(Str_View(other))
	{
	}
	Simple_Short_String(Simple_Short_String&& other) noexcept
	    : sz(other.sz), is_long(other.is_long)
	{
		if (is_short()) {
			traits_type::copy(short_data, other.short_data, sz);
			short_data[sz] = {};
			return;
		}
		long_data = other.long_data;
		other.sz = 0;
		other.is_long = false;
		other.short_data[0] = {};
	}
	auto operator=(const Simple_Short_String&)
	    -> Simple_Short_String& = delete;
	auto operator=(Simple_Short_String &&) -> Simple_Short_String& = delete;

	auto size() const noexcept { return sz; }
};
using Short_WString = Simple_Short_String<wchar_t>;

/**
 * @brief Vector of strings that recycles erased strings
 */
template <class CharT>
class List_Basic_Strings {
	using Vec_Str = std::vector<std::basic_string<CharT>>;
	Vec_Str d;
	size_t sz = 0;

      public:
	using value_type = typename Vec_Str::value_type;
	using allocator_type = typename Vec_Str::allocator_type;
	using size_type = typename Vec_Str::size_type;
	using difference_type = typename Vec_Str::difference_type;
	using reference = typename Vec_Str::reference;
	using const_reference = typename Vec_Str::const_reference;
	using pointer = typename Vec_Str::pointer;
	using const_pointer = typename Vec_Str::const_pointer;
	using iterator = typename Vec_Str::iterator;
	using const_iterator = typename Vec_Str::const_iterator;
	using reverse_iterator = typename Vec_Str::reverse_iterator;
	using const_reverse_iterator = typename Vec_Str::const_reverse_iterator;

	List_Basic_Strings() = default;
	explicit List_Basic_Strings(size_type n) : d(n), sz(n) {}
	List_Basic_Strings(size_type n, const_reference value)
	    : d(n, value), sz(n)
	{
	}
	template <class InputIterator>
	List_Basic_Strings(InputIterator first, InputIterator last)
	    : d(first, last), sz(d.size())
	{
	}
	List_Basic_Strings(std::initializer_list<value_type> il)
	    : d(il), sz(d.size())
	{
	}

	List_Basic_Strings(const List_Basic_Strings& other) = default;
	List_Basic_Strings(List_Basic_Strings&& other) noexcept
	    : d(move(other.d)), sz(other.sz)
	{
		other.sz = 0;
	}

	List_Basic_Strings(Vec_Str&& other) : d(other), sz(other.size()) {}

	auto& operator=(const List_Basic_Strings& other)
	{
		clear();
		insert(begin(), other.begin(), other.end());
		return *this;
	}
	auto& operator=(List_Basic_Strings&& other) noexcept
	{
		d = move(other.d);
		sz = other.sz;
		other.sz = 0;
		return *this;
	}
	auto& operator=(std::initializer_list<value_type> il)
	{
		clear();
		insert(begin(), il);
		return *this;
	}
	template <class InputIterator>
	auto assign(InputIterator first, InputIterator last)
	{
		clear();
		insert(begin(), first, last);
	}
	void assign(size_type n, const_reference value)
	{
		clear();
		insert(begin(), n, value);
	}
	void assign(std::initializer_list<value_type> il) { *this = il; }
	auto get_allocator() const noexcept { return d.get_allocator(); }

	// iterators
	auto begin() noexcept -> iterator { return d.begin(); }
	auto begin() const noexcept -> const_iterator { return d.begin(); }
	auto end() noexcept -> iterator { return begin() + sz; }
	auto end() const noexcept -> const_iterator { return begin() + sz; }

	auto rbegin() noexcept { return d.rend() - sz; }
	auto rbegin() const noexcept { return d.rend() - sz; }
	auto rend() noexcept { return d.rend(); }
	auto rend() const noexcept { return d.rend(); }

	auto cbegin() const noexcept { return d.cbegin(); }
	auto cend() const noexcept { return cbegin() + sz; }

	auto crbegin() const noexcept { return d.crend() - sz; }
	auto crend() const noexcept { return d.crend(); }

	// [vector.capacity], capacity
	auto empty() const noexcept { return sz == 0; }
	auto size() const noexcept { return sz; }
	auto max_size() const noexcept { return d.max_size(); }
	auto capacity() const noexcept { return d.size(); }
	auto resize(size_type new_sz)
	{
		if (new_sz <= sz) {
		}
		else if (new_sz <= d.size()) {
			std::for_each(begin() + sz, begin() + new_sz,
			              [](auto& s) { s.clear(); });
		}
		else {
			std::for_each(d.begin() + sz, d.end(),
			              [](auto& s) { s.clear(); });
			d.resize(new_sz);
		}
		sz = new_sz;
	}
	auto resize(size_type new_sz, const_reference c)
	{
		if (new_sz <= sz) {
		}
		else if (new_sz <= d.size()) {
			std::for_each(begin() + sz, begin() + new_sz,
			              [&](auto& s) { s = c; });
		}
		else {
			std::for_each(d.begin() + sz, d.end(),
			              [&](auto& s) { s = c; });
			d.resize(new_sz, c);
		}
		sz = new_sz;
	}
	void reserve(size_type n)
	{
		if (n > d.size())
			d.resize(n);
	}
	void shrink_to_fit()
	{
		d.resize(sz);
		d.shrink_to_fit();
		for (auto& s : d) {
			s.shrink_to_fit();
		}
	}

	// element access
	auto& operator[](size_type n) { return d[n]; }
	auto& operator[](size_type n) const { return d[n]; }
	auto& at(size_type n)
	{
		if (n < sz)
			return d[n];
		else
			throw std::out_of_range("index is out of range");
	}
	auto& at(size_type n) const
	{
		if (n < sz)
			return d[n];
		else
			throw std::out_of_range("index is out of range");
	}
	auto& front() { return d.front(); }
	auto& front() const { return d.front(); }
	auto& back() { return d[sz - 1]; }
	auto& back() const { return d[sz - 1]; }

	// [vector.data], data access
	auto data() noexcept { return d.data(); }
	auto data() const noexcept { return d.data(); }

	// [vector.modifiers], modifiers
	template <class... Args>
	auto& emplace_back(Args&&... args)
	{
		if (sz != d.size())
			d[sz] = value_type(std::forward<Args>(args)...);
		else
			d.emplace_back(std::forward<Args>(args)...);
		return d[sz++];
	}
	auto& emplace_back()
	{
		if (sz != d.size())
			d[sz].clear();
		else
			d.emplace_back();
		return d[sz++];
	}
	auto push_back(const_reference x)
	{
		if (sz != d.size())
			d[sz] = x;
		else
			d.push_back(x);
		++sz;
	}
	auto push_back(value_type&& x)
	{
		if (sz != d.size())
			d[sz] = std::move(x);
		else
			d.push_back(std::move(x));
		++sz;
	}
	auto pop_back() { --sz; }

      private:
	template <class U>
	auto insert_priv(const_iterator pos, U&& val)
	{
		if (sz != d.size()) {
			d[sz] = std::forward<U>(val);
		}
		else {
			auto pos_idx = pos - cbegin();
			d.push_back(std::forward<U>(val));
			pos = cbegin() + pos_idx;
		}
		auto p = begin() + (pos - cbegin());
		std::rotate(p, begin() + sz, begin() + sz + 1);
		++sz;
		return p;
	}

      public:
	template <class... Args>
	auto emplace(const_iterator pos, Args&&... args)
	{
		if (sz != d.size()) {
			d[sz] = value_type(std::forward<Args>(args)...);
		}
		else {
			auto pos_idx = pos - cbegin();
			d.emplace(std::forward<Args>(args)...);
			pos = cbegin() + pos_idx;
		}
		auto p = begin() + (pos - cbegin());
		std::rotate(p, begin() + sz, begin() + sz + 1);
		++sz;
		return p;
	}
	auto insert(const_iterator pos, const_reference x)
	{
		return insert_priv(pos, x);
	}
	auto insert(const_iterator pos, value_type&& x)
	{
		return insert_priv(pos, std::move(x));
	}
	auto insert(const_iterator pos, size_type n, const_reference x)
	    -> iterator
	{
		auto i = sz;
		while (n != 0 && i != d.size()) {
			d[i] = x;
			--n;
			++i;
		}
		if (n != 0) {
			auto pos_idx = pos - cbegin();
			d.insert(d.end(), n, x);
			pos = cbegin() + pos_idx;
			i = d.size();
		}
		auto p = begin() + (pos - cbegin());
		std::rotate(p, begin() + sz, begin() + i);
		sz = i;
		return p;
	}

	template <class InputIterator>
	auto insert(const_iterator pos, InputIterator first, InputIterator last)
	    -> iterator
	{
		auto i = sz;
		while (first != last && i != d.size()) {
			d[i] = *first;
			++first;
			++i;
		}
		if (first != last) {
			auto pos_idx = pos - cbegin();
			d.insert(d.end(), first, last);
			pos = cbegin() + pos_idx;
			i = d.size();
		}
		auto p = begin() + (pos - cbegin());
		std::rotate(p, begin() + sz, begin() + i);
		sz = i;
		return p;
	}
	auto insert(const_iterator pos, std::initializer_list<value_type> il)
	    -> iterator
	{
		return insert(pos, il.begin(), il.end());
	}

	auto erase(const_iterator position)
	{
		auto i0 = begin();
		auto i1 = i0 + (position - cbegin());
		auto i2 = i1 + 1;
		auto i3 = end();
		std::rotate(i1, i2, i3);
		--sz;
		return i1;
	}
	auto erase(const_iterator first, const_iterator last)
	{
		auto i0 = begin();
		auto i1 = i0 + (first - cbegin());
		auto i2 = i0 + (last - cbegin());
		auto i3 = end();
		std::rotate(i1, i2, i3);
		sz -= last - first;
		return i1;
	}
	auto swap(List_Basic_Strings& other)
	{
		d.swap(other.d);
		std::swap(sz, other.sz);
	}
	auto clear() noexcept -> void { sz = 0; }

	auto operator==(const List_Basic_Strings& other) const
	{
		return std::equal(begin(), end(), other.begin(), other.end());
	}
	auto operator!=(const List_Basic_Strings& other) const
	{
		return !(*this == other);
	}
	auto operator<(const List_Basic_Strings& other) const
	{
		return std::lexicographical_compare(begin(), end(),
		                                    other.begin(), other.end());
	}
	auto operator>=(const List_Basic_Strings& other) const
	{
		return !(*this < other);
	}
	auto operator>(const List_Basic_Strings& other) const
	{
		return std::lexicographical_compare(other.begin(), other.end(),
		                                    begin(), end());
	}
	auto operator<=(const List_Basic_Strings& other) const
	{
		return !(*this > other);
	}

	auto extract_sequence() -> Vec_Str
	{
		d.resize(sz);
		sz = 0;
		return std::move(d);
	}
};

template <class CharT>
auto swap(List_Basic_Strings<CharT>& a, List_Basic_Strings<CharT>& b)
{
	a.swap(b);
}

using List_Strings = List_Basic_Strings<char>;
using List_WStrings = List_Basic_Strings<wchar_t>;

template <class CharT>
class Replacement_Table {
      public:
	using Str = std::basic_string<CharT>;
	using Table_Str = std::vector<std::pair<Str, Str>>;
	using iterator = typename Table_Str::iterator;
	using const_iterator = typename Table_Str::const_iterator;

      private:
	Table_Str table;
	size_t whole_word_reps_last_idx = 0;
	size_t start_word_reps_last_idx = 0;
	size_t end_word_reps_last_idx = 0;

	auto order_entries() -> void;

      public:
	Replacement_Table() = default;
	Replacement_Table(const Table_Str& v) : table(v) { order_entries(); }
	Replacement_Table(Table_Str&& v) : table(move(v)) { order_entries(); }

	auto& operator=(const Table_Str& v)
	{
		table = v;
		order_entries();
		return *this;
	}

	auto& operator=(Table_Str&& v)
	{
		table = move(v);
		order_entries();
		return *this;
	}

	auto whole_word_replacements() const
	    -> boost::iterator_range<const_iterator>
	{
		return {begin(table), begin(table) + whole_word_reps_last_idx};
	}
	auto start_word_replacements() const
	    -> boost::iterator_range<const_iterator>
	{
		return {begin(table) + whole_word_reps_last_idx,
		        begin(table) + start_word_reps_last_idx};
	}
	auto end_word_replacements() const
	    -> boost::iterator_range<const_iterator>
	{
		return {begin(table) + start_word_reps_last_idx,
		        begin(table) + end_word_reps_last_idx};
	}
	auto any_place_replacements() const
	    -> boost::iterator_range<const_iterator>
	{
		return {begin(table) + end_word_reps_last_idx, end(table)};
	}
};
template <class CharT>
auto Replacement_Table<CharT>::order_entries() -> void
{
	auto it = remove_if(begin(table), end(table), [](auto& p) {
		auto& s = p.first;
		return s.empty() ||
		       (s.size() == 1 && (s[0] == '^' || s[0] == '$'));
	});
	table.erase(it, end(table));

	auto is_start_word_pat = [=](auto& x) { return x.first[0] == '^'; };
	auto is_end_word_pat = [=](auto& x) { return x.first.back() == '$'; };

	auto start_word_reps_last =
	    partition(begin(table), end(table), is_start_word_pat);
	start_word_reps_last_idx = start_word_reps_last - begin(table);
	for_each(begin(table), start_word_reps_last,
	         [](auto& e) { e.first.erase(0, 1); });

	auto whole_word_reps_last =
	    partition(begin(table), start_word_reps_last, is_end_word_pat);
	whole_word_reps_last_idx = whole_word_reps_last - begin(table);
	for_each(begin(table), whole_word_reps_last,
	         [](auto& e) { e.first.pop_back(); });

	auto end_word_reps_last =
	    partition(start_word_reps_last, end(table), is_end_word_pat);
	end_word_reps_last_idx = end_word_reps_last - begin(table);
	for_each(start_word_reps_last, end_word_reps_last,
	         [](auto& e) { e.first.pop_back(); });
}

template <class CharT>
struct Similarity_Group {
	using Str = std::basic_string<CharT>;
	Str chars;
	std::vector<Str> strings;

	auto parse(const Str& s) -> void;
	Similarity_Group() = default;
	Similarity_Group(const Str& s) { parse(s); }
	auto& operator=(const Str& s)
	{
		parse(s);
		return *this;
	}
};
template <class CharT>
auto Similarity_Group<CharT>::parse(const Str& s) -> void
{
	auto i = size_t(0);
	for (;;) {
		auto j = s.find('(', i);
		chars.append(s, i, j - i);
		if (j == s.npos)
			break;
		i = j + 1;
		j = s.find(')', i);
		if (j == s.npos)
			break;
		auto len = j - i;
		if (len == 1)
			chars += s[i];
		else if (len > 1)
			strings.push_back(s.substr(i, len));
		i = j + 1;
	}
}

template <class CharT>
class Phonetic_Table {
	using Str = std::basic_string<CharT>;
	using Pair_Str = std::pair<Str, Str>;

	struct Phonet_Match_Result {
		size_t count_matched = 0;
		size_t go_back_before_replace = 0;
		size_t priority = 5;
		bool go_back_after_replace = false;
		bool treat_next_as_begin = false;
		operator bool() { return count_matched; }
	};

	std::vector<std::pair<Str, Str>> table;
	auto order() -> void;
	auto static match(const Str& data, size_t i, const Str& pattern,
	                  bool at_begin) -> Phonet_Match_Result;

      public:
	Phonetic_Table() = default;
	Phonetic_Table(const std::vector<Pair_Str>& v) : table(v) { order(); }
	Phonetic_Table(std::vector<Pair_Str>&& v) : table(move(v)) { order(); }
	auto& operator=(const std::vector<Pair_Str>& v)
	{
		table = v;
		order();
		return *this;
	}
	auto& operator=(std::vector<Pair_Str>&& v)
	{
		table = move(v);
		order();
		return *this;
	}
	auto replace(Str& word) const -> bool;
};

template <class CharT>
auto Phonetic_Table<CharT>::order() -> void
{
	stable_sort(begin(table), end(table), [](auto& pair1, auto& pair2) {
		if (pair2.first.empty())
			return false;
		if (pair1.first.empty())
			return true;
		return pair1.first[0] < pair2.first[0];
	});
	auto it = find_if_not(begin(table), end(table),
	                      [](auto& p) { return p.first.empty(); });
	table.erase(begin(table), it);
	for (auto& r : table) {
		if (r.second == NUSPELL_LITERAL(CharT, "_"))
			r.second.clear();
	}
}

template <class CharT>
auto Phonetic_Table<CharT>::match(const Str& data, size_t i, const Str& pattern,
                                  bool at_begin) -> Phonet_Match_Result
{
	auto ret = Phonet_Match_Result();
	auto j =
	    pattern.find_first_of(NUSPELL_LITERAL(CharT, "(<-0123456789^$"));
	if (j == pattern.npos)
		j = pattern.size();
	if (data.compare(i, j, pattern, 0, j) == 0)
		ret.count_matched = j;
	else
		return {};
	if (j == pattern.size())
		return ret;
	if (pattern[j] == '(') {
		auto k = pattern.find(')', j);
		if (k == pattern.npos)
			return {}; // bad rule
		auto x = std::char_traits<CharT>::find(
		    &pattern[j + 1], k - (j + 1), data[i + j]);
		if (!x)
			return {};
		j = k + 1;
		ret.count_matched += 1;
	}
	if (j == pattern.size())
		return ret;
	if (pattern[j] == '<') {
		ret.go_back_after_replace = true;
		++j;
	}
	auto k = pattern.find_first_not_of('-', j);
	if (k == pattern.npos) {
		k = pattern.size();
		ret.go_back_before_replace = k - j;
		if (ret.go_back_before_replace >= ret.count_matched)
			return {}; // bad rule
		return ret;
	}
	else {
		ret.go_back_before_replace = k - j;
		if (ret.go_back_before_replace >= ret.count_matched)
			return {}; // bad rule
	}
	j = k;
	if (pattern[j] >= '0' && pattern[j] <= '9') {
		ret.priority = pattern[j] - '0';
		++j;
	}
	if (j == pattern.size())
		return ret;
	if (pattern[j] == '^') {
		if (!at_begin)
			return {};
		++j;
	}
	if (j == pattern.size())
		return ret;
	if (pattern[j] == '^') {
		ret.treat_next_as_begin = true;
		++j;
	}
	if (j == pattern.size())
		return ret;
	if (pattern[j] != '$')
		return {}; // bad rule, no other char is allowed at this point
	if (i + ret.count_matched == data.size())
		return ret;
	return {};
}

template <class CharT>
auto Phonetic_Table<CharT>::replace(Str& word) const -> bool
{
	using boost::make_iterator_range;
	struct Cmp {
		auto operator()(CharT c, const Pair_Str& s)
		{
			return c < s.first[0];
		}
		auto operator()(const Pair_Str& s, CharT c)
		{
			return s.first[0] < c;
		}
	};
	if (table.empty())
		return false;
	auto ret = false;
	auto treat_next_as_begin = true;
	size_t count_go_backs_after_replace = 0; // avoid infinite loop
	for (size_t i = 0; i != word.size(); ++i) {
		auto rules =
		    equal_range(begin(table), end(table), word[i], Cmp());
		for (auto& r : make_iterator_range(rules)) {
			auto rule = &r;
			auto m1 = match(word, i, r.first, treat_next_as_begin);
			if (!m1)
				continue;
			if (!m1.go_back_before_replace) {
				auto j = i + m1.count_matched - 1;
				auto rules2 = equal_range(
				    begin(table), end(table), word[j], Cmp());
				for (auto& r2 : make_iterator_range(rules2)) {
					auto m2 =
					    match(word, j, r2.first, false);
					if (m2 && m2.priority >= m1.priority) {
						i = j;
						rule = &r2;
						m1 = m2;
						break;
					}
				}
			}
			word.replace(
			    i, m1.count_matched - m1.go_back_before_replace,
			    rule->second);
			treat_next_as_begin = m1.treat_next_as_begin;
			if (m1.go_back_after_replace &&
			    count_go_backs_after_replace < 100) {
				count_go_backs_after_replace++;
			}
			else {
				i += rule->second.size();
			}
			--i;
			ret = true;
			break;
		}
	}
	return ret;
}
} // namespace v3
} // namespace nuspell
#endif // NUSPELL_STRUCTURES_HXX
