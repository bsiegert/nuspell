/* Copyright 2016-2018 Dimitrij Mijoski
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

#ifndef NUSPELL_DICTIONARY_HXX
#define NUSPELL_DICTIONARY_HXX

#include "aff_data.hxx"
#include "locale_utils.hxx"

#include <fstream>
#include <locale>

#include <boost/optional.hpp>

namespace nuspell {

enum Spell_Result {
	BAD_WORD /**< word is spelled incorrectly */,
	GOOD_WORD /**< word is spelled correctly */,
	AFFIXED_GOOD_WORD /**< affixed word is spelled correctly */,
	COMPOUND_GOOD_WORD /**< compound word is spelled correctly */
};

enum Affixing_Mode {
	FULL_WORD,
	AT_COMPOUND_BEGIN,
	AT_COMPOUND_END,
	AT_COMPOUND_MIDDLE
};

class Dictionary : protected Aff_Data {
      private:
	template <class CharT>
	auto spell_priv(std::basic_string<CharT> s) -> Spell_Result;
	template <class CharT>
	auto spell_break(std::basic_string<CharT> s) -> Spell_Result;
	template <class CharT>
	auto spell_casing(std::basic_string<CharT> s) -> Spell_Result;
	template <class CharT>
	auto spell_casing_upper(std::basic_string<CharT> s) -> const Flag_Set*;
	template <class CharT>
	auto spell_casing_title(std::basic_string<CharT> s) -> const Flag_Set*;
	template <class CharT>
	auto spell_sharps(std::basic_string<CharT> base, size_t n_pos = 0,
	                  size_t n = 0, size_t rep = 0) -> const Flag_Set*;
	template <class CharT>
	auto checkword(std::basic_string<CharT>& s) const -> const Flag_Set*;

	/**
	 * @brief strip_prefix_only
	 * @param s derived word with affixes
	 * @return if found, root word + prefix
	 */
	template <Affixing_Mode m = FULL_WORD, class CharT>
	auto strip_prefix_only(std::basic_string<CharT>& s) const
	    -> boost::optional<
	        std::tuple<std::basic_string<CharT>, const Flag_Set&,
	                   const Prefix<CharT>&>>;

	/**
	 * @brief strip_suffix_only
	 * @param s derived word with affixes
	 * @return if found, root word + suffix
	 */
	template <Affixing_Mode m = FULL_WORD, class CharT>
	auto strip_suffix_only(std::basic_string<CharT>& s) const
	    -> boost::optional<
	        std::tuple<std::basic_string<CharT>, const Flag_Set&,
	                   const Suffix<CharT>&>>;

	/**
	 * @brief strip_prefix_then_suffix
	 *
	 * This accepts a derived word that was formed first by adding
	 * suffix then prefix to the root. The stripping is in reverse.
	 *
	 * @param s derived word with affixes
	 * @return if found, root word + suffix + prefix
	 */
	template <Affixing_Mode m = FULL_WORD, class CharT>
	auto strip_prefix_then_suffix(std::basic_string<CharT>& s) const
	    -> boost::optional<
	        std::tuple<std::basic_string<CharT>, const Flag_Set&,
	                   const Suffix<CharT>&, const Prefix<CharT>&>>;

	template <Affixing_Mode m = FULL_WORD, class CharT>
	auto strip_pfx_then_sfx_2(const Prefix<CharT>& pe,
	                          std::basic_string<CharT>& s) const
	    -> boost::optional<
	        std::tuple<std::basic_string<CharT>, const Flag_Set&,
	                   const Suffix<CharT>&, const Prefix<CharT>&>>;

	/**
	 * @brief strip_suffix_then_prefix
	 *
	 * This accepts a derived word that was formed first by adding
	 * prefix then suffix to the root. The stripping is in reverse.
	 *
	 * @param s derived word with prefix and suffix
	 * @return if found, root word + prefix + suffix
	 */
	template <Affixing_Mode m = FULL_WORD, class CharT>
	auto strip_suffix_then_prefix(std::basic_string<CharT>& s) const
	    -> boost::optional<
	        std::tuple<std::basic_string<CharT>, const Flag_Set&,
	                   const Prefix<CharT>&, const Suffix<CharT>&>>;

	template <Affixing_Mode m = FULL_WORD, class CharT>
	auto strip_sfx_then_pfx_2(const Suffix<CharT>& se,
	                          std::basic_string<CharT>& s) const
	    -> boost::optional<
	        std::tuple<std::basic_string<CharT>, const Flag_Set&,
	                   const Prefix<CharT>&, const Suffix<CharT>&>>;

	template <Affixing_Mode m = FULL_WORD, class CharT>
	auto strip_suffix_then_suffix(std::basic_string<CharT>& s)
	    -> boost::optional<
	        std::tuple<std::basic_string<CharT>, const Flag_Set&,
	                   const Suffix<CharT>&, const Suffix<CharT>&>>;

	template <Affixing_Mode m = FULL_WORD, class CharT>
	auto strip_prefix_then_prefix(std::basic_string<CharT>& s)
	    -> boost::optional<
	        std::tuple<std::basic_string<CharT>, const Flag_Set&,
	                   const Prefix<CharT>&, const Prefix<CharT>&>>;

      public:
	Dictionary()
	    : Aff_Data() // we explicity do value init so content is zeroed
	{
	}
	explicit Dictionary(const string& dict_file_path) : Aff_Data()
	{
		std::ifstream aff_file(dict_file_path + ".aff");
		std::ifstream dic_file(dict_file_path + ".dic");
		parse(aff_file, dic_file);
		// Set to false to disable logging.
		if (true)
			log(dict_file_path + ".aff");
	}

	auto spell_dict_encoding(const std::string& word) -> Spell_Result;

	auto spell_c_locale(const std::string& word) -> Spell_Result;

	auto spell(const std::string& word, std::locale loc = std::locale())
	    -> Spell_Result
	{
		using namespace std;
		using info_t = boost::locale::info;
		auto& dic_info = use_facet<info_t>(locale_aff);
		Locale_Input l;
		if (dic_info.utf8()) {
			return spell_priv<wchar_t>(
			    l.cvt_for_u8_dict(word, loc));
		}
		else {
			return spell_priv<char>(
			    l.cvt_for_byte_dict(word, loc, locale_aff));
		}
	}
	auto spell_u8(const std::string& word) -> Spell_Result;
	auto spell(const std::wstring& word) -> Spell_Result;
	auto spell(const std::u16string& word) -> Spell_Result;
	auto spell(const std::u32string& word) -> Spell_Result;
};
}
#endif // NUSPELL_DICTIONARY_HXX
