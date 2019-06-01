#include <emscripten/bind.h>
#include <vector>
#include <anitomy/anitomy.h>

using namespace emscripten;
using namespace anitomy;
using namespace std;

Elements ParseSingle(string_t filename)
{
    Anitomy an;
    an.Parse(filename);
    return an.elements();
}

vector<Elements> ParseMultiple(vector<string_t> filenames)
{
    Anitomy an;
    vector<Elements> elements;
    for (auto filename : filenames)
    {
        an.Parse(filename);
        elements.emplace_back(an.elements());
    }
    return elements;
}

vector<string_t> *StringVectorFromPointer(uintptr_t vec)
{
    return reinterpret_cast<vector<string_t> *>(vec);
}

vector<Elements> *ElementVectorFromPointer(uintptr_t vec)
{
    return reinterpret_cast<vector<Elements> *>(vec);
}

EMSCRIPTEN_BINDINGS(Anitomy)
{
    emscripten::function("parseSingle", &ParseSingle);
    emscripten::function("parseMultiple", &ParseMultiple);

    register_vector<string_t>("StringVector")
        .constructor(&StringVectorFromPointer, allow_raw_pointers());

    register_vector<Elements>("ElementVector")
        .constructor(&ElementVectorFromPointer, allow_raw_pointers());

    class_<Elements>("Elements")
        .constructor<>()
        .function("size", &Elements::size)
        .function("empty_capacity", select_overload<bool() const>(&Elements::empty))
        .function("empty_lookup", select_overload<bool(ElementCategory) const>(&Elements::empty))
        .function("get", &Elements::get)
        .function("count", &Elements::count)
        .function("get_all", &Elements::get_all);

    enum_<ElementCategory>("ElementCategory")
        .value("kElementIterateFirst", ElementCategory::kElementIterateFirst)
        .value("kElementAnimeSeason", ElementCategory::kElementAnimeSeason)
        .value("kElementAnimeSeasonPrefix", ElementCategory::kElementAnimeSeasonPrefix)
        .value("kElementAnimeTitle", ElementCategory::kElementAnimeTitle)
        .value("kElementAnimeType", ElementCategory::kElementAnimeType)
        .value("kElementAnimeYear", ElementCategory::kElementAnimeYear)
        .value("kElementAudioTerm", ElementCategory::kElementAudioTerm)
        .value("kElementDeviceCompatibility", ElementCategory::kElementDeviceCompatibility)
        .value("kElementEpisodeNumber", ElementCategory::kElementEpisodeNumber)
        .value("kElementEpisodeNumberAlt", ElementCategory::kElementEpisodeNumberAlt)
        .value("kElementEpisodePrefix", ElementCategory::kElementEpisodePrefix)
        .value("kElementEpisodeTitle", ElementCategory::kElementEpisodeTitle)
        .value("kElementFileChecksum", ElementCategory::kElementFileChecksum)
        .value("kElementFileExtension", ElementCategory::kElementFileExtension)
        .value("kElementFileName", ElementCategory::kElementFileName)
        .value("kElementLanguage", ElementCategory::kElementLanguage)
        .value("kElementOther", ElementCategory::kElementOther)
        .value("kElementReleaseGroup", ElementCategory::kElementReleaseGroup)
        .value("kElementReleaseInformation", ElementCategory::kElementReleaseInformation)
        .value("kElementReleaseVersion", ElementCategory::kElementReleaseVersion)
        .value("kElementSource", ElementCategory::kElementSource)
        .value("kElementSubtitles", ElementCategory::kElementSubtitles)
        .value("kElementVideoResolution", ElementCategory::kElementVideoResolution)
        .value("kElementVideoTerm", ElementCategory::kElementVideoTerm)
        .value("kElementVolumeNumber", ElementCategory::kElementVolumeNumber)
        .value("kElementVolumePrefix", ElementCategory::kElementVolumePrefix)
        .value("kElementIterateLast", ElementCategory::kElementIterateLast)
        .value("kElementUnknown", ElementCategory::kElementUnknown);
}
