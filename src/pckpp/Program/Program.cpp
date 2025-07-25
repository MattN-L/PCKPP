﻿#include "Program.h"
#include "../Application/Application.h"
#include "../UI/Tree/TreeFunctions.h"
#include "../UI/Menu/MenuFunctions.h"
#include <map>
#include <sstream>

void ProgramSetup() {
	gApp->SetFolderIcon(gApp->GetGraphics()->LoadTextureFromFile("assets/icons/NODE_FOLDER.png", TextureFilter::LINEAR_MIPMAP_LINEAR));
	for (int i = 0; i < (int)PCKAssetFile::Type::PCK_ASSET_TYPES_TOTAL; i++) {
		auto type = static_cast<PCKAssetFile::Type>(i);
		std::string name = (type == PCKAssetFile::Type::UI_DATA)
			? PCKAssetFile::getAssetTypeString(PCKAssetFile::Type::PCK_ASSET_TYPES_TOTAL)
			: PCKAssetFile::getAssetTypeString(type);
		std::string path = "assets/icons/FILE_" + name + ".png";
		gApp->SetFileIcon(type, gApp->GetGraphics()->LoadTextureFromFile(path, TextureFilter::LINEAR_MIPMAP_LINEAR));
	}
}

void ResetProgramData() {
	gApp->GetInstance()->Reset();
}

static void ProgramCleanup() {
	ResetProgramData();
}

void HandleMenuBar() {
	gApp->GetUI()->RenderMenuBar();
}

void HandleInput()
{
	// tbh, I'm not really sure where the hell this should go; UI it is lol
	gApp->GetUI()->HandleInput();
}

// Renders and handles window to preview the currently selected file if any data is previewable
static void HandlePreviewWindow(const PCKAssetFile& file) {
	gApp->GetUI()->RenderPreviewWindow(file);
}

static void HandlePropertiesWindow(const PCKAssetFile& file)
{
	gApp->GetUI()->RenderPropertiesWindow(file);
}

void HandleFileTree() {
	BuildFileTree();
	gApp->GetUI()->RenderFileTree();
}