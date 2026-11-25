// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception
// Copyright (C) 2020 Jesse Chappell


#include "JuceHeader.h"
#include "VersionInfo.h"

std::unique_ptr<VersionInfo> VersionInfo::fetchFromUpdateServer (const String& versionString)
{
    return fetch ("tags/" + versionString);
}

std::unique_ptr<VersionInfo> VersionInfo::fetchLatestFromUpdateServer()
{
    return fetch ("latest");
}

std::unique_ptr<InputStream> VersionInfo::createInputStreamForAsset (const Asset& asset, int& statusCode)
{
    URL downloadUrl (asset.url);
    StringPairArray responseHeaders;
    String extraHeaders;   
    extraHeaders += "Accept: application/octet-stream\r\n";
    
    String authuserpass = SystemStats::getEnvironmentVariable("GITUSERPASS", "");    
    if (authuserpass.isNotEmpty()) {
        extraHeaders += "Authorization: Basic " + Base64::toBase64(authuserpass) + "\r\n";
    }
    
    ;
    
    return std::unique_ptr<InputStream> (downloadUrl.createInputStream (URL::InputStreamOptions(URL::ParameterHandling::inAddress).withExtraHeaders(extraHeaders).withResponseHeaders(&responseHeaders).withStatusCode(&statusCode).withNumRedirectsToFollow(1)));
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

std::unique_ptr<VersionInfo> VersionInfo::fetch (const String&)
{
    return nullptr;
}
