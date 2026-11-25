// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception
// Copyright (C) 2020 Jesse Chappell


#include "JuceHeader.h"
#include "VersionInfo.h"

std::unique_ptr<VersionInfo> VersionInfo::fetchFromUpdateServer (const String& versionString)
{
    return fetch ("tags/v" + versionString);
}

std::unique_ptr<VersionInfo> VersionInfo::fetchLatestFromUpdateServer()
{
    return fetch ("latest");
}

std::unique_ptr<InputStream> VersionInfo::createInputStreamForAsset (const Asset& asset, int& statusCode)
{
    URL downloadUrl (asset.url);
    return downloadUrl.createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)
                                              .withStatusCode (&statusCode));
}

bool VersionInfo::isNewerVersionThanCurrent()
{
    jassert (versionString.isNotEmpty());

    auto currentTokens = StringArray::fromTokens (ProjectInfo::versionString, ".", {});
    auto thisTokens    = StringArray::fromTokens (versionString, ".", {});

    jassert (thisTokens.size() >= 2 && thisTokens.size() >= 2);

    if (currentTokens[0].getIntValue() == thisTokens[0].getIntValue())
    {
        // check for 'b' in second token
        if (thisTokens[1].contains("b")) {            
            return currentTokens[1].compareIgnoreCase(thisTokens[1]); 
        }
        else {
            if (currentTokens[1].getIntValue() == thisTokens[1].getIntValue()) {
                if (currentTokens.size() > 2 && thisTokens.size() > 2) {
                    return currentTokens[2].getIntValue() < thisTokens[2].getIntValue();
                } else if (thisTokens.size() > 2) {
                    return true;
                } else {
                    return false;
                }
            }
            
            return currentTokens[1].getIntValue() < thisTokens[1].getIntValue();
        }
    }

    return currentTokens[0].getIntValue() < thisTokens[0].getIntValue();
}

std::unique_ptr<VersionInfo> VersionInfo::fetch (const String& endpoint)
{
    const URL releaseUrl ("https://api.github.com/repos/arWai-CW/SonoRoom/releases/" + endpoint);
    int statusCode = 0;
    auto stream = releaseUrl.createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)
                                                   .withExtraHeaders ("User-Agent: SonoRoom\r\nAccept: application/vnd.github+json\r\n")
                                                   .withConnectionTimeoutMs (5000)
                                                   .withStatusCode (&statusCode));
    if (stream == nullptr || statusCode != 200)
        return nullptr;

    const auto release = JSON::parse (stream->readEntireStreamAsString());
    auto* details = release.getDynamicObject();
    if (details == nullptr)
        return nullptr;

    const auto tag = details->getProperty ("tag_name").toString();
    if (! tag.startsWithChar ('v'))
        return nullptr;

    const auto version = tag.substring (1);
    const auto parts = StringArray::fromTokens (version, ".", {});
    if (parts.size() != 3)
        return nullptr;

    for (const auto& part : parts)
        if (part.isEmpty() || ! part.containsOnly ("0123456789"))
            return nullptr;

    std::vector<Asset> assets;
    const auto releaseAssets = details->getProperty ("assets");
    if (const auto* entries = releaseAssets.getArray())
        for (const auto& entry : *entries)
            if (auto* asset = entry.getDynamicObject())
                if (asset->getProperty ("name").toString() == "SonoRoom-mac.dmg")
                    assets.push_back ({ "SonoRoom-mac.dmg", "https://github.com/arWai-CW/SonoRoom/releases/download/" + tag + "/SonoRoom-mac.dmg" });

    return std::unique_ptr<VersionInfo> (new VersionInfo ({ version, details->getProperty ("body").toString(), std::move (assets) }));
}
