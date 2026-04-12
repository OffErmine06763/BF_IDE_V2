#pragma once
#include "State.h"
#include "Utility.h"

#include <filesystem>


class SelectProjectState : public State
{
public:
	struct Entry
	{
		u32 ID;
		PathType Type;
		fs::path Path;

		Entry(const u32 id, const fs::path& path)
			: ID(id), Type(GetPathType(path)), Path(path)
		{ }
	};

public:
	SelectProjectState();
	~SelectProjectState() override;

	void Render() override;

private:
	void RenderFav();
	void RenderRec();

	void CacheHistory();

	std::vector<Entry> Fav, Rec;

	u32 CurrId = 0;

	bool m_DraggingRec = false, m_DraggingFav = false;
	bool request_move_from_rec = false, request_move_from_fav = false;
};