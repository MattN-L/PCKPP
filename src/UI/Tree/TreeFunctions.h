#pragma once

#include "../UI.h"
#include "../../PCK/PCKAssetFile.h"

struct FileTreeNode {
	std::string path{};
	PCKAssetFile* file{ nullptr }; // folder by default
	std::vector<FileTreeNode> children;
};

void SavePCK(std::vector<FileTreeNode> nodes, IO::Endianness endianness, const std::string& path = "", const std::string& defaultName = "");

void SaveFolderAsFiles(const FileTreeNode& node, bool includeProperties = false);

// Convert file tree to PCK File collection
void TreeToPCKFileCollection(std::vector<FileTreeNode>& treeNodes);

// Finds a node by path in a given file tree
FileTreeNode* FindNodeByPath(const std::string& path, std::vector<FileTreeNode>& nodes);

// Deletes a node in a given file tree
void DeleteNode(FileTreeNode& targetNode, std::vector<FileTreeNode>& nodes);

// Sorts a given file tree
void SortTree(FileTreeNode& node);

// Builds a given file tree
void BuildFileTree(std::vector<FileTreeNode>& nodes);

// Scrolls to selected node when not visible
void ScrollToNode(bool& keyScrolled);