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

	//struct Node
	//{
	//	std::shared_ptr<Node>
	//		Next = nullptr,
	//		Prev = nullptr,
	//		NextRec = nullptr, // TODO: union tra prev e tra next?
	//		NextFav = nullptr,
	//		PrevRec = nullptr,
	//		PrevFav = nullptr;

	//	Entry Data;

	//	Node(const Entry& e) : Data(e) {}
	//};
	//struct Unnamed
	//{
	//	std::shared_ptr<Node> Root = nullptr, Tail = nullptr;
	//	size_t Size = 0, RecSize = 0, FavSize = 0;

	//	void AddRec(const uint32_t id, const std::filesystem::path& path);
	//	void AddFav(const uint32_t id, const std::filesystem::path& path);
	//};

public:
	SelectProjectState();
	~SelectProjectState() override;

	void Render() override;

private:
	void RenderFav();
	void RenderRec();

	void CacheHistory();

	// Unnamed _Unnamed;
	std::vector<Entry> Fav, Rec;

	u32 CurrId = 0;
};