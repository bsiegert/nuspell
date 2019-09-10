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
 * @brief Dictionary spelling, PUBLIC HEADER.
 */

#ifndef NUSPELL_DICTIONARY_HXX
#define NUSPELL_DICTIONARY_HXX

#include "aff_data.hxx"

namespace nuspell {

enum Affixing_Mode {
	FULL_WORD,
	AT_COMPOUND_BEGIN,
	AT_COMPOUND_END,
	AT_COMPOUND_MIDDLE
};

template <class... Affixes>
struct Affixing_Result
    : public std::tuple<Word_List::const_pointer, const Affixes*...> {
	using base_type =
	    std::tuple<Word_List::const_pointer, const Affixes*...>;
	Affixing_Result() = default;
	Affixing_Result(Word_List::const_reference word_entry,
	                const Affixes&... affixes)
	    : base_type(&word_entry, &affixes...)
	{
	}
	// operator bool() const { return std::get<0>(*this); }
	operator Word_List::const_pointer() const { return std::get<0>(*this); }
	auto& operator*() const { return *std::get<0>(*this); }
	auto operator-> () const { return std::get<0>(*this); }
};

struct Compounding_Result {
	Word_List::const_pointer word_entry = {};
	bool affixed_and_modified = {}; /**< non-zero affix */
	operator Word_List::const_pointer() const { return word_entry; }
	auto& operator*() const { return *word_entry; }
	auto operator-> () const { return word_entry; }
};

struct Dict_Base : public Aff_Data {

	auto spell_priv(std::wstring& s) const -> bool;
	auto spell_break(std::wstring& s, size_t depth = 0) const -> bool;
	auto spell_casing(std::wstring& s) const -> const Flag_Set*;
	auto spell_casing_upper(std::wstring& s) const -> const Flag_Set*;
	auto spell_casing_title(std::wstring& s) const -> const Flag_Set*;
	auto spell_sharps(std::wstring& base, size_t n_pos = 0, size_t n = 0,
	                  size_t rep = 0) const -> const Flag_Set*;

	auto check_word(std::wstring& s) const -> const Flag_Set*;

	template <Affixing_Mode m>
	auto affix_NOT_valid(const Prefix<wchar_t>& a) const;
	template <Affixing_Mode m>
	auto affix_NOT_valid(const Suffix<wchar_t>& a) const;
	template <Affixing_Mode m, class AffixT>
	auto outer_affix_NOT_valid(const AffixT& a) const;
	template <class AffixT>
	auto is_circumfix(const AffixT& a) const;
	template <Affixing_Mode m>
	auto is_valid_inside_compound(const Flag_Set& flags) const;

	/**
	 * @brief strip_prefix_only
	 * @param s derived word with affixes
	 * @return if found, root word + prefix
	 */
	template <Affixing_Mode m = FULL_WORD>
	auto strip_prefix_only(std::wstring& s) const
	    -> Affixing_Result<Prefix<wchar_t>>;

	/**
	 * @brief strip_suffix_only
	 * @param s derived word with affixes
	 * @return if found, root word + suffix
	 */
	template <Affixing_Mode m = FULL_WORD>
	auto strip_suffix_only(std::wstring& s) const
	    -> Affixing_Result<Suffix<wchar_t>>;

	/**
	 * @brief strip_prefix_then_suffix
	 *
	 * This accepts a derived word that was formed first by adding
	 * suffix then prefix to the root. The stripping is in reverse.
	 *
	 * @param s derived word with affixes
	 * @return if found, root word + suffix + prefix
	 */
	template <Affixing_Mode m = FULL_WORD>
	auto strip_prefix_then_suffix(std::wstring& s) const
	    -> Affixing_Result<Suffix<wchar_t>, Prefix<wchar_t>>;

	template <Affixing_Mode m>
	auto strip_pfx_then_sfx_2(const Prefix<wchar_t>& pe,
	                          std::wstring& s) const
	    -> Affixing_Result<Suffix<wchar_t>, Prefix<wchar_t>>;

	/**
	 * @brief strip_suffix_then_prefix
	 *
	 * This accepts a derived word that was formed first by adding
	 * prefix then suffix to the root. The stripping is in reverse.
	 *
	 * @param s derived word with prefix and suffix
	 * @return if found, root word + prefix + suffix
	 */
	template <Affixing_Mode m = FULL_WORD>
	auto strip_suffix_then_prefix(std::wstring& s) const
	    -> Affixing_Result<Prefix<wchar_t>, Suffix<wchar_t>>;

	template <Affixing_Mode m>
	auto strip_sfx_then_pfx_2(const Suffix<wchar_t>& se,
	                          std::wstring& s) const
	    -> Affixing_Result<Prefix<wchar_t>, Suffix<wchar_t>>;

	template <Affixing_Mode m = FULL_WORD>
	auto strip_prefix_then_suffix_commutative(std::wstring& word) const
	    -> Affixing_Result<Suffix<wchar_t>, Prefix<wchar_t>>;

	template <Affixing_Mode m = FULL_WORD>
	auto strip_pfx_then_sfx_comm_2(const Prefix<wchar_t>& pe,
	                               std::wstring& word) const
	    -> Affixing_Result<Suffix<wchar_t>, Prefix<wchar_t>>;

	template <Affixing_Mode m = FULL_WORD>
	auto strip_suffix_then_suffix(std::wstring& s) const
	    -> Affixing_Result<Suffix<wchar_t>, Suffix<wchar_t>>;

	template <Affixing_Mode m>
	auto strip_sfx_then_sfx_2(const Suffix<wchar_t>& se1,
	                          std::wstring& s) const
	    -> Affixing_Result<Suffix<wchar_t>, Suffix<wchar_t>>;

	template <Affixing_Mode m = FULL_WORD>
	auto strip_prefix_then_prefix(std::wstring& s) const
	    -> Affixing_Result<Prefix<wchar_t>, Prefix<wchar_t>>;

	template <Affixing_Mode m>
	auto strip_pfx_then_pfx_2(const Prefix<wchar_t>& pe1,
	                          std::wstring& s) const
	    -> Affixing_Result<Prefix<wchar_t>, Prefix<wchar_t>>;

	template <Affixing_Mode m = FULL_WORD>
	auto strip_prefix_then_2_suffixes(std::wstring& s) const
	    -> Affixing_Result<>;

	template <Affixing_Mode m>
	auto strip_pfx_2_sfx_3(const Prefix<wchar_t>& pe1,
	                       const Suffix<wchar_t>& se1,
	                       std::wstring& s) const -> Affixing_Result<>;

	template <Affixing_Mode m = FULL_WORD>
	auto strip_suffix_prefix_suffix(std::wstring& s) const
	    -> Affixing_Result<>;

	template <Affixing_Mode m>
	auto strip_s_p_s_3(const Suffix<wchar_t>& se1,
	                   const Prefix<wchar_t>& pe1, std::wstring& word) const
	    -> Affixing_Result<>;

	template <Affixing_Mode m = FULL_WORD>
	auto strip_2_suffixes_then_prefix(std::wstring& s) const
	    -> Affixing_Result<>;

	template <Affixing_Mode m>
	auto strip_2_sfx_pfx_3(const Suffix<wchar_t>& se1,
	                       const Suffix<wchar_t>& se2,
	                       std::wstring& word) const -> Affixing_Result<>;

	template <Affixing_Mode m = FULL_WORD>
	auto strip_suffix_then_2_prefixes(std::wstring& s) const
	    -> Affixing_Result<>;

	template <Affixing_Mode m>
	auto strip_sfx_2_pfx_3(const Suffix<wchar_t>& se1,
	                       const Prefix<wchar_t>& pe1,
	                       std::wstring& s) const -> Affixing_Result<>;

	template <Affixing_Mode m = FULL_WORD>
	auto strip_prefix_suffix_prefix(std::wstring& word) const
	    -> Affixing_Result<>;

	template <Affixing_Mode m>
	auto strip_p_s_p_3(const Prefix<wchar_t>& pe1,
	                   const Suffix<wchar_t>& se1, std::wstring& word) const
	    -> Affixing_Result<>;

	template <Affixing_Mode m = FULL_WORD>
	auto strip_2_prefixes_then_suffix(std::wstring& word) const
	    -> Affixing_Result<>;
	template <Affixing_Mode m>
	auto strip_2_pfx_sfx_3(const Prefix<wchar_t>& pe1,
	                       const Prefix<wchar_t>& pe2,
	                       std::wstring& word) const -> Affixing_Result<>;

	template <Affixing_Mode m = FULL_WORD>
	auto strip_pfx_then_2_sfx_comm(std::wstring& s) const
	    -> Affixing_Result<>;

	auto check_compound(std::wstring& word) const -> Compounding_Result;

	template <Affixing_Mode m = AT_COMPOUND_BEGIN>
	auto check_compound(std::wstring& word, size_t start_pos,
	                    size_t num_part, std::wstring& part) const
	    -> Compounding_Result;

	template <Affixing_Mode m = AT_COMPOUND_BEGIN>
	auto check_compound_classic(std::wstring& word, size_t start_pos,
	                            size_t i, size_t num_part,
	                            std::wstring& part) const
	    -> Compounding_Result;

	template <Affixing_Mode m = AT_COMPOUND_BEGIN>
	auto check_compound_with_pattern_replacements(std::wstring& word,
	                                              size_t start_pos,
	                                              size_t i, size_t num_part,
	                                              std::wstring& part) const
	    -> Compounding_Result;

	auto check_compound_with_rules(std::wstring& word,
	                               std::vector<const Flag_Set*>& words_data,
	                               size_t start_pos,
	                               std::wstring& part) const
	    -> Compounding_Result;

	template <Affixing_Mode m>
	auto check_word_in_compound(std::wstring& s) const
	    -> Compounding_Result;

	auto suggest_priv(std::wstring& word, List_WStrings& out) const -> void;

	auto add_sug_if_correct(std::wstring& word, List_WStrings& out) const
	    -> bool;

	auto try_rep_suggestion(std::wstring& word, List_WStrings& out) const
	    -> void;

	auto rep_suggest(std::wstring& word, List_WStrings& out) const -> void;

	auto extra_char_suggest(std::wstring& word, List_WStrings& out) const
	    -> void;

	auto map_suggest(std::wstring& word, List_WStrings& out,
	                 size_t i = 0) const -> void;

	auto keyboard_suggest(std::wstring& word, List_WStrings& out) const
	    -> void;

	auto bad_char_suggest(std::wstring& word, List_WStrings& out) const
	    -> void;

	auto forgotten_char_suggest(std::wstring& word,
	                            List_WStrings& out) const -> void;

	auto phonetic_suggest(std::wstring& word, List_WStrings& out) const
	    -> void;

      public:
	Dict_Base()
	    : Aff_Data() // we explicity do value init so content is zeroed
	{
	}
};

/**
 * @brief Public API is inline namespace
 */
inline namespace v2 {

class Dictionary_Loading_Error : public std::runtime_error {
      public:
	using std::runtime_error::runtime_error;
};

/**
 * @brief The only important public class
 */
class Dictionary : private Dict_Base {
	std::locale external_locale;
	bool external_locale_known_utf8;

	Dictionary(std::istream& aff, std::istream& dic);
	auto external_to_internal_encoding(const std::string& in,
	                                   std::wstring& wide_out) const
	    -> bool;

	auto internal_to_external_encoding(const std::wstring& wide_in,
	                                   std::string& out) const -> bool;

      public:
	Dictionary();
	auto static load_from_aff_dic(std::istream& aff, std::istream& dic)
	    -> Dictionary;
	auto static load_from_path(
	    const std::string& file_path_without_extension) -> Dictionary;
	auto imbue(const std::locale& loc) -> void;
	auto spell(const std::string& word) const -> bool;
	auto suggest(const std::string& word,
	             std::vector<std::string>& out) const -> void;
};
} // namespace v2
} // namespace nuspell
#endif // NUSPELL_DICTIONARY_HXX
