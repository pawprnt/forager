#pragma once

#include "providers/Provider.h"
#include <vector>

class SteamLibrary {
public:
    std::vector<OwnedGame> fetchOwnedGames() const;
    std::vector<OwnedGame> fetchInstalledGames() const;

private:
    static bool parseOwnedResponse(const QByteArray& data, std::vector<OwnedGame>& out);
};
