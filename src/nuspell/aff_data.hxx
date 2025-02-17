/* Copyright 2016-2019 Dimitrij Mijoski
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
 * @brief Affixing data structures, private header.
 */

#ifndef NUSPELL_AFF_DATA_HXX
#define NUSPELL_AFF_DATA_HXX

#include "structures.hxx"

#include <iosfwd>
#include <unicode/locid.h>

namespace nuspell {
inline namespace v3 {

class Encoding {
	std::string name;

	auto normalize_name() -> void;

      public:
	enum Enc_Type { SINGLEBYTE = false, UTF8 = true };

	Encoding() = default;
	Encoding(const std::string& e) : name(e) { normalize_name(); }
	Encoding(std::string&& e) : name(move(e)) { normalize_name(); }
	Encoding(const char* e) : name(e) { normalize_name(); }
	auto& operator=(const std::string& e)
	{
		name = e;
		normalize_name();
		return *this;
	}
	auto& operator=(std::string&& e)
	{
		name = move(e);
		normalize_name();
		return *this;
	}
	auto& operator=(const char* e)
	{
		name = e;
		normalize_name();
		return *this;
	}
	auto empty() const { return name.empty(); }
	operator const std::string&() const { return name; }
	auto& value() const { return name; }
	auto is_utf8() const { return name == "UTF-8"; }
	auto value_or_default() const -> std::string
	{
		if (name.empty())
			return "ISO8859-1";
		else
			return name;
	}
	operator Enc_Type() const { return is_utf8() ? UTF8 : SINGLEBYTE; }
};

enum class Flag_Type {
	SINGLE_CHAR /**< single-character flag, e.g. for "a" */,
	DOUBLE_CHAR /**< double-character flag, e.g for "aa" */,
	NUMBER /**< numerical flag, e.g. for 61 */,
	UTF8 /**< UTF-8 flag, e.g. for "á" */
};

struct Extractor_First_of_Word_Pair {
	auto& operator()(const std::pair<std::wstring, Flag_Set>& p) const
	{
		return p.first;
	}
};

/**
 * @brief Map between words and word_flags.
 *
 * Flags are stored as part of the container. Maybe for the future flags should
 * be stored elsewhere (flag aliases) and this should store pointers.
 *
 * Does not store morphological data as is low priority feature and is out of
 * scope.
 */
using Word_List = Hash_Multiset<std::pair<std::wstring, Flag_Set>, std::wstring,
                                Extractor_First_of_Word_Pair>;

struct Aff_Data {
	static const auto HIDDEN_HOMONYM_FLAG = char16_t(-1);

	// spell checking options
	Word_List words;
	Prefix_Table prefixes;
	Suffix_Table suffixes;

	bool complex_prefixes;
	bool fullstrip;
	bool checksharps;
	bool forbid_warn;
	char16_t compound_onlyin_flag;
	char16_t circumfix_flag;
	char16_t forbiddenword_flag;
	char16_t keepcase_flag;
	char16_t need_affix_flag;
	char16_t warn_flag;

	// compounding options
	char16_t compound_flag;
	char16_t compound_begin_flag;
	char16_t compound_last_flag;
	char16_t compound_middle_flag;
	Compound_Rule_Table compound_rules;

	// spell checking options
	Break_Table<wchar_t> break_table;
	Substr_Replacer<wchar_t> input_substr_replacer;
	std::wstring ignored_chars;
	icu::Locale icu_locale;
	Substr_Replacer<wchar_t> output_substr_replacer;

	// suggestion options
	Replacement_Table<wchar_t> replacements;
	std::vector<Similarity_Group<wchar_t>> similarities;
	std::wstring keyboard_closeness;
	std::wstring try_chars;
	Phonetic_Table<wchar_t> phonetic_table;

	char16_t nosuggest_flag;
	char16_t substandard_flag;
	unsigned short max_compound_suggestions;
	unsigned short max_ngram_suggestions;
	unsigned short max_diff_factor;
	bool only_max_diff;
	bool no_split_suggestions;
	bool suggest_with_dots;

	// compounding options
	unsigned short compound_min_length;
	unsigned short compound_max_word_count;
	char16_t compound_permit_flag;
	char16_t compound_forbid_flag;
	char16_t compound_root_flag;
	char16_t compound_force_uppercase;
	bool compound_more_suffixes;
	bool compound_check_duplicate;
	bool compound_check_rep;
	bool compound_check_case;
	bool compound_check_triple;
	bool compound_simplified_triple;
	bool compound_syllable_num;
	unsigned short compound_syllable_max;
	std::wstring compound_syllable_vowels;
	std::vector<Compound_Pattern<wchar_t>> compound_patterns;

	// data members used only while parsing
	Flag_Type flag_type;
	Encoding encoding;
	std::vector<Flag_Set> flag_aliases;
	std::string wordchars; // deprecated?

	auto parse_aff(std::istream& in) -> bool;
	auto parse_dic(std::istream& in) -> bool;
	auto parse_aff_dic(std::istream& aff, std::istream& dic)
	{
		if (parse_aff(aff))
			return parse_dic(dic);
		return false;
	}
};
} // namespace v3
} // namespace nuspell

#endif // NUSPELL_AFF_DATA_HXX
