#pragma once
#include "State.h"
#include "Utility.h"

#include <filesystem>


class SelectProjectState : public State
{
public:
	struct Entry
	{
		const uint32_t ID;
		const PathType Type;
		const std::filesystem::path Path;
	};

public:
	SelectProjectState();
	~SelectProjectState() override;

	void Render() override;

private:
	void RenderFav();
	void RenderRec();

	void CacheHistory();

	std::vector<Entry> Fav, Recent;
	uint32_t CurrId = 0;
};