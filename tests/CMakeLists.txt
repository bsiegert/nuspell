add_executable(unit_test
    aff_data_test.cxx
    dictionary_test.cxx
    structures_test.cxx
    utils_test.cxx
    catch_main.cxx)
target_link_libraries(unit_test nuspell Catch2::Catch2)
if (MSVC)
    target_compile_options(unit_test PRIVATE "/utf-8")
    # Consider doing this for all the other targets by setting this flag
    # globally for MSVC. ATM we use unicode string literals only in the tests.
endif()

add_executable(legacy_test legacy_test.cxx)
target_link_libraries(legacy_test nuspell)

add_executable(verify verify.cxx)
target_link_libraries(verify nuspell hunspell Boost::locale)

if (BUILD_SHARED_LIBS AND WIN32)
    add_custom_command(TARGET unit_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:nuspell> $<TARGET_FILE_DIR:unit_test>)
endif()

include(Catch)
catch_discover_tests(unit_test)

file(GLOB v1tests
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/v1cmdline
    "v1cmdline/*.dic"
    "v1cmdline/*.sug")
foreach(t ${v1tests})
    add_test(
        NAME ${t}
        COMMAND legacy_test ${CMAKE_CURRENT_SOURCE_DIR}/v1cmdline/${t})
endforeach()

set_tests_properties(
base_utf.dic
nepali.dic
1463589.sug
1463589_utf.sug
allcaps.sug
allcaps_utf.sug
base.sug
base_utf.sug
checksharps.sug
checksharpsutf.sug
forceucase.sug
i35725.sug
i54633.sug
keepcase.sug
nosuggest.sug
oconv.sug
opentaal_keepcase.sug
phone.sug
utf8_nonbmp.sug

PROPERTIES WILL_FAIL TRUE)
