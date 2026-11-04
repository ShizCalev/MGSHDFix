#include "stdafx.h"
#include "unit_tests.hpp"

#include "config_keys.hpp"

#if defined(RELEASE_BUILD)
void UnitTests::runAllTests()
{
    
}
#else
#undef NDEBUG

#include "logging.hpp"
#include "version_checking.hpp"

namespace
{
    void testCompareSemVer()
    {
        assert(VersionCheck::CompareSemanticVersion("1.0.0", "1.0.0") == VersionCheck::CompareResult::Equal);
        assert(VersionCheck::CompareSemanticVersion("1.0.0", "1.0.0.0") == VersionCheck::CompareResult::Equal);
        assert(VersionCheck::CompareSemanticVersion("1.0.0.0", "1.0") == VersionCheck::CompareResult::Equal);
        assert(VersionCheck::CompareSemanticVersion("01.02.03", "1.2.3") == VersionCheck::CompareResult::Equal);
        assert(VersionCheck::CompareSemanticVersion("", "0.0.0") == VersionCheck::CompareResult::Equal);

        assert(VersionCheck::CompareSemanticVersion("1.2a", "1.2.1") == VersionCheck::CompareResult::Equal);
        assert(VersionCheck::CompareSemanticVersion("1.2a", "1.2.1.0") == VersionCheck::CompareResult::Equal);
        assert(VersionCheck::CompareSemanticVersion("1.2b", "1.2.2.0") == VersionCheck::CompareResult::Equal);
        assert(VersionCheck::CompareSemanticVersion("1.2b", "1.2.2") == VersionCheck::CompareResult::Equal);

        assert(VersionCheck::CompareSemanticVersion("0.1.0", "0.0.9") == VersionCheck::CompareResult::Newer);
        assert(VersionCheck::CompareSemanticVersion("1.0.0", "0.0.9") == VersionCheck::CompareResult::Newer);
        assert(VersionCheck::CompareSemanticVersion("1.0.0", "0.0.9.9") == VersionCheck::CompareResult::Newer);
        assert(VersionCheck::CompareSemanticVersion("1.0.0.1", "1.0") == VersionCheck::CompareResult::Newer);
        assert(VersionCheck::CompareSemanticVersion("1.10.0", "1.9.999") == VersionCheck::CompareResult::Newer);
        assert(VersionCheck::CompareSemanticVersion("1.2b", "1.2a") == VersionCheck::CompareResult::Newer);
        assert(VersionCheck::CompareSemanticVersion("2.0.0", "1.999.999.999") == VersionCheck::CompareResult::Newer);
        assert(VersionCheck::CompareSemanticVersion("65535.65535.65535.65535", "65535.65535.65535.65534") == VersionCheck::CompareResult::Newer);

        assert(VersionCheck::CompareSemanticVersion("garbage", "1.0.0") == VersionCheck::CompareResult::Older);
        assert(VersionCheck::CompareSemanticVersion("1.0.0", "1.0.1") == VersionCheck::CompareResult::Older);
        assert(VersionCheck::CompareSemanticVersion("1.0.0", "1.0.1.1") == VersionCheck::CompareResult::Older);
        assert(VersionCheck::CompareSemanticVersion("1.1.0", "1.1a") == VersionCheck::CompareResult::Older);
        assert(VersionCheck::CompareSemanticVersion("1.2", "1.2.1") == VersionCheck::CompareResult::Older);
        assert(VersionCheck::CompareSemanticVersion("1.2a", "1.2.1.1") == VersionCheck::CompareResult::Older);
        assert(VersionCheck::CompareSemanticVersion("1.2a", "1.2b") == VersionCheck::CompareResult::Older);

        spdlog::info("Unit Test: compareSemVersion completed successfully.");
    }

    void testResolveApplicableVersion()
    {
        std::string sVersion;

        assert(VersionCheck::ResolveApplicableVersion("2.0.0", "MGS2", sVersion) == true && sVersion == "2.0.0");
        assert(VersionCheck::ResolveApplicableVersion("v2.0.0", "MSX", sVersion) == true && sVersion == "2.0.0");

        assert(VersionCheck::ResolveApplicableVersion("1.0.0-MGS2", "MGS2", sVersion) == true && sVersion == "1.0.0");
        assert(VersionCheck::ResolveApplicableVersion("1.0.0-mgs3", "MGS3", sVersion) == true && sVersion == "1.0.0");
        assert(VersionCheck::ResolveApplicableVersion("v1.2.3-MSX-MGS3", "MSX", sVersion) == true && sVersion == "1.2.3");
        assert(VersionCheck::ResolveApplicableVersion("1.2.3-MSX-MGS3", "MGS3", sVersion) == true && sVersion == "1.2.3");

        assert(VersionCheck::ResolveApplicableVersion("1.2.3-MSX-MGS3", "MGS2", sVersion) == false);
        assert(VersionCheck::ResolveApplicableVersion("1.0.0-MGS2", "MSX", sVersion) == false);
        assert(VersionCheck::ResolveApplicableVersion("1.0.0-MGS2", "", sVersion) == false);

        spdlog::info("Unit Test: testResolveApplicableVersion completed successfully.");
    }

    void testLanguagePairs()
    {
// ==========================================================
// MG1 / MG2 / MGS2
// ==========================================================

// Valid pairs
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "eu", "en") == true);
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "eu", "fr") == true);
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "eu", "it") == true);
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "eu", "gr") == true);
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "eu", "sp") == true);
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "jp", "jp") == true);

        // Invalid region for valid language
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "us", "en") == false);
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "us", "fr") == false);
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "us", "jp") == false);

        // Invalid language for valid region
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "eu", "jp") == false);
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "jp", "en") == false);
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "jp", "fr") == false);

        // Fully invalid
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "us", "sp") == false);
        assert(IsValidRegionLanguagePair(MG1_MG2_MGS2_LanguagePairs, "kr", "en") == false);


        // ==========================================================
        // MGS3
        // ==========================================================

        // Valid North America
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "us", "en") == true);
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "us", "fr") == true);
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "us", "sp") == true);

        // Valid Europe
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "eu", "en") == true);
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "eu", "fr") == true);
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "eu", "it") == true);
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "eu", "gr") == true);
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "eu", "sp") == true);

        // Valid Japan
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "jp", "jp") == true);

        // Invalid language for region
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "us", "jp") == false);
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "us", "it") == false);
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "us", "gr") == false);
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "eu", "jp") == false);
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "jp", "en") == false);
        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "jp", "fr") == false);

        assert(IsValidRegionLanguagePair(MGS3_LanguagePairs, "kr", "en") == false);

        SPDLOG_INFO("Unit Test : testLanguagePairs completed successfully.");

    }




}




void UnitTests::runAllTests()
{
    testCompareSemVer();

    testResolveApplicableVersion();

    testLanguagePairs();

}

#define NDEBUG  // NOLINT(clang-diagnostic-unused-macros)
#endif
