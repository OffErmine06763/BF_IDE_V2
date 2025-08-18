#pragma once
#include "Utility.h"
#include "Tool.h"
#include "Events/Event.h"


// TODO: Tree view add white/black list based on extension, search, expand to current file

class TreeTool : public Tool
{
public:
	struct TreeEntry
	{
		fs::path Path;
		bool Directory = false;
		bool Collapsed = true;
		std::vector<TreeEntry> Children;

		/// maps a path in the cached m_TreeEntries to it's index in the vector
		hmap<fs::path, size_t> Map; // TODO: replace with a tree structure, each node extends the parent path and contains the index
	};

public:
	TreeTool(const fs::path& root);
	~TreeTool() override = default;

	void Render() override;

	inline std::string Name() const override { return "Tree"; }
	inline Type GetType() const override { return _GetType(); }
	static inline Type _GetType() { return TREE; }

	listener_id SubscribeSelect(consumer<const fs::path&> cb) { return m_SelectEvent.Subscribe(cb); }
	listener_id SubscribeCompile(consumer<const fs::path&> cb) { return m_CompileEvent.Subscribe(cb); }
	listener_id SubscribeDelete(consumer<const fs::path&> cb) { return m_DeleteEvent.Subscribe(cb); }
	listener_id SubscribeNew(consumer<const fs::path&> cb) { return m_NewEvent.Subscribe(cb); }

private:
	void CacheDirectoryTree(TreeEntry& parent);
	void RenderTreeEntry(TreeEntry& entry);


private:
	TreeEntry m_TreeRoot;
	u32 m_TreeCacheCounter = 0;

	Event<const fs::path&> m_SelectEvent, m_CompileEvent, m_DeleteEvent, m_NewEvent;
};