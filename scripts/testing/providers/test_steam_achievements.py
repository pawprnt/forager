import pathlib

from forager.providers.steam.achievements import (
    achievement_summary,
    parse_vdf,
    player_achievements,
)

_SAMPLE = """
"achievements"
{
\t"0" { "name" "First Blood" "path" "icons/0.jpg" "achieved" "1" }
\t"1" { "name" "Sharpshooter" "path" "icons/1.jpg" "achieved" "0" }
}
"""


def test_parse_vdf_structure():
    data = parse_vdf(_SAMPLE)
    assert list(data.keys()) == ["achievements"]
    inner = data["achievements"]
    assert inner["0"]["name"] == "First Blood"
    assert inner["0"]["achieved"] == "1"


def test_player_achievements_missing(tmp_path):
    assert player_achievements("1", "440", steam_root=tmp_path) == []


def test_player_achievements_parses(tmp_path):
    vdf = tmp_path / "userdata" / "123" / "440" / "achievements.vdf"
    vdf.parent.mkdir(parents=True)
    vdf.write_text(_SAMPLE)
    res = player_achievements("123", "440", steam_root=tmp_path)
    assert len(res) == 2
    assert res[0]["name"] == "First Blood"
    assert res[0]["achieved"] is True
    assert res[1]["achieved"] is False
    earned, total, frac = achievement_summary(res)
    assert (earned, total, frac) == (1, 2, 0.5)
