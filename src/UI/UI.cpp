﻿#include "../Application/Application.h"
#include "UI.h"
#include "Tree/TreeFunctions.h"

// Resource globals
std::map<PCKAssetFile::Type, Texture> gFileIcons;
std::vector<FileTreeNode> gTreeNodes;
std::vector<const FileTreeNode*> gVisibleNodes;
Texture gFolderIcon;

// Preview globals
Texture gPreviewTexture{};
std::string gPreviewTitle = "Preview";
static const PCKAssetFile* gLastPreviewedFile = nullptr;

// Instance globals
static std::string gSelectedNodePath;
static bool gHasXMLSupport{ false };
static IO::Endianness gPCKEndianness{ IO::Endianness::LITTLE };

void UISetup() {
	SDL_Surface* icon = SDL_LoadBMP("assets/icons/ICON_PCKPP.bmp");
	if (icon) {
		SDL_SetWindowIcon(GetWindow(), icon);
		SDL_DestroySurface(icon);
	}

	gFolderIcon = LoadTextureFromFile("assets/icons/NODE_FOLDER.png", GL_LINEAR_MIPMAP_LINEAR);
	for (int i = 0; i < (int)PCKAssetFile::Type::PCK_ASSET_TYPES_TOTAL; i++) {
		auto type = static_cast<PCKAssetFile::Type>(i);
		std::string name = (type == PCKAssetFile::Type::UI_DATA)
			? PCKAssetFile::getAssetTypeString(PCKAssetFile::Type::PCK_ASSET_TYPES_TOTAL)
			: PCKAssetFile::getAssetTypeString(type);
		std::string path = "assets/icons/FILE_" + name + ".png";
		gFileIcons[type] = LoadTextureFromFile(path, GL_LINEAR_MIPMAP_LINEAR);
	}

	ImGuiStyle& style = ImGui::GetStyle();
	style.CellPadding = ImVec2(0, 0);

	ImFontConfig config;
	config.MergeMode = false;
	config.PixelSnapH = true;

	ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/m6x11plus.ttf", 18.0f, &config);

	config.MergeMode = true;

	// Merge Chinese (Simplified)
	ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/ark-pixel-12px-monospaced-zh_cn.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
	// Merge Chinese (Traditional)
	ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/ark-pixel-12px-monospaced-zh_tw.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
	// Merge Japanese
	ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/ark-pixel-12px-monospaced-ja.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
	// Merge Korean
	ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/ark-pixel-12px-monospaced-ko.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesKorean());

	ImGui::GetIO().Fonts->Build();
}

void UICleanup() {
	ResetUIData();
	for (auto& [type, tex] : gFileIcons)
		if (tex.id != 0)
			glDeleteTextures(1, &tex.id);
	gFileIcons.clear();
}

void ResetUIData(const std::string& filePath) {

	PCKFile* pckFile = gApp->CurrentPCKFile();

	if (pckFile)
	{
		gHasXMLSupport = pckFile->getXMLSupport();
		gPCKEndianness = pckFile->getEndianness();
	}

	gSelectedNodePath = "";
	gVisibleNodes.clear();
	gTreeNodes.clear();
}

void HandleMenuBar() {
	PCKFile* pckFile = gApp->CurrentPCKFile();

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open", "Ctrl+O")) {
				OpenPCKFileDialog();
			}
			if (pckFile)
			{
				if (ImGui::MenuItem("Save", "Ctrl+S", nullptr, pckFile)) {
					SavePCK(gTreeNodes, gPCKEndianness, pckFile->getFilePath());
				}
				if (ImGui::MenuItem("Save as", "Ctrl+Shift+S", nullptr, pckFile)) {
					SavePCK(gTreeNodes, gPCKEndianness, "", pckFile->getFileName());
				}
			}
			ImGui::EndMenu();
		}

		if (pckFile)
		{
			if (ImGui::BeginMenu("PCK"))
			{
				ImGui::Text("PCK Format:");
				if (ImGui::RadioButton("Little Endian (Xbox One, PS4, PSVita, Nintendo Switch)", gPCKEndianness == IO::Endianness::LITTLE))
				{
					gPCKEndianness = IO::Endianness::LITTLE;
				}

				if (ImGui::RadioButton("Big Endian (Xbox 360, PS3, Wii U)", gPCKEndianness == IO::Endianness::BIG))
				{
					gPCKEndianness = IO::Endianness::BIG;
				}

				ImGui::NewLine();
				if (ImGui::Checkbox("Full BOX Support (for Skins)", &gHasXMLSupport)) {
					pckFile->setXMLSupport(gHasXMLSupport);
				}

				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();
	}
}

void HandleInput()
{
	PCKFile* pckFile = gApp->CurrentPCKFile();

	// make sure to pass false or else it will trigger multiple times
	if (pckFile && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
		if (ShowYesNoMessagePrompt("Are you sure?", "This is permanent and cannot be undone.\nIf this is a folder, all sub-files will be deleted too.")) {
			if (FileTreeNode* node = FindNodeByPath(gSelectedNodePath, gTreeNodes))
			{
				DeleteNode(*node, gTreeNodes);
				TreeToPCKFileCollection(gTreeNodes);
			}
		}
		else
			ShowCancelledMessage();
	}

	if (ImGui::GetIO().KeyCtrl)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_O, false)) {
			OpenPCKFileDialog();
		}
		else if (pckFile && ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
			SavePCK(gTreeNodes, gPCKEndianness, pckFile->getFilePath()); // Save
		}
		else if (pckFile && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
			SavePCK(gTreeNodes, gPCKEndianness, "", pckFile->getFileName()); // Save As
		}
	}
}

void ShowSuccessMessage()
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Success", "Operation performed successfully.", GetWindow());
}

void ShowCancelledMessage()
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Cancelled", "User aborted operation.", GetWindow());
}

static int ShowMessagePrompt(const char* title, const char* message, const SDL_MessageBoxButtonData* buttons, int numButtons)
{
	static SDL_MessageBoxData messageboxdata = {};
	messageboxdata.flags = SDL_MESSAGEBOX_WARNING | SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;
	messageboxdata.title = title;
	messageboxdata.message = message;
	messageboxdata.numbuttons = numButtons;
	messageboxdata.buttons = buttons;
	messageboxdata.window = GetWindow();

	int buttonID = -1;
	if (SDL_ShowMessageBox(&messageboxdata, &buttonID)) {
		return buttonID; // return the button ID user clicked
	}
	else {
		SDL_Log("Failed to show message box: %s", SDL_GetError());
		return -1; // indicate error
	}
}

static bool ShowYesNoMessagePrompt(const char* title, const char* message)
{
	const static SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No" }
	};

	return ShowMessagePrompt(title, message, buttons, SDL_arraysize(buttons)) == 1;
}

void ResetPreviewWindow()
{
	glDeleteTextures(1, &gPreviewTexture.id);
	gPreviewTexture = {};
	gLastPreviewedFile = nullptr;
}

// Renders and handles window to preview the currently selected file if any data is previewable
static void HandlePreviewWindow(const PCKAssetFile& file) {
	static bool zoomChanged = false;
	static float userZoom = 1.0f;

	// if ID is valid AND last file is not the current file
	if (gPreviewTexture.id != 0 && gLastPreviewedFile != &file) {
		ResetPreviewWindow();
		zoomChanged = false;
		userZoom = 1.0f;
	}

	if (gLastPreviewedFile != &file) {
		gPreviewTexture = LoadTextureFromMemory(file.getData().data(), file.getFileSize());
		gLastPreviewedFile = &file;
		gPreviewTitle = file.getPath() + " (" + std::to_string(gPreviewTexture.width) + "x" + std::to_string(gPreviewTexture.height) + ")###Preview";

		userZoom = 1.0f;
		zoomChanged = false;
	}

	if (gPreviewTexture.id == 0) return;

	float previewPosX = ImGui::GetIO().DisplaySize.x * 0.25f;
	ImVec2 previewWindowSize(ImGui::GetIO().DisplaySize.x * 0.75f, ImGui::GetIO().DisplaySize.y - (ImGui::GetIO().DisplaySize.y * 0.35f));
	ImGui::SetNextWindowPos(ImVec2(previewPosX, ImGui::GetFrameHeight()), ImGuiCond_Always);
	ImGui::SetNextWindowSize(previewWindowSize, ImGuiCond_Always);

	ImGui::Begin(gPreviewTitle.c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::BeginChild("PreviewScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	if (ImGui::IsWindowHovered() && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f) {
		float zoomDelta = ImGui::GetIO().MouseWheel * 0.1f;
		userZoom = std::clamp(userZoom * (1.0f + zoomDelta), 0.01f, 100.0f); // this clamp is a little weird but it works lol
		zoomChanged = true;
	}

	ImVec2 availSize = ImGui::GetContentRegionAvail();

	if (!zoomChanged && userZoom == 1.0f) {
		userZoom = std::min(
			(availSize.x) / gPreviewTexture.width,
			(availSize.y) / gPreviewTexture.height
		);
	}

	ImVec2 imageSize = ImVec2(gPreviewTexture.width * userZoom, gPreviewTexture.height * userZoom);

	ImVec2 cursorPos = ImGui::GetCursorPos();
	if (imageSize.x < availSize.x) cursorPos.x += (availSize.x - imageSize.x) / 2.0f;
	if (imageSize.y < availSize.y) cursorPos.y += (availSize.y - imageSize.y) / 2.0f;
	ImGui::SetCursorPos(cursorPos);
	ImGui::Image((ImTextureID)(intptr_t)gPreviewTexture.id, imageSize);

	std::stringstream ss;
	ss << "Zoom: " << std::fixed << std::setprecision(2) << (userZoom * 100.0f) << "%";
	std::string zoomText = ss.str();
	ImVec2 textSize = ImGui::CalcTextSize(zoomText.c_str());
	ImVec2 textPos = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - textSize.x - 20, ImGui::GetWindowPos().y);
	ImGui::SetCursorScreenPos(textPos);
	ImGui::TextUnformatted(zoomText.c_str());

	ImGui::EndChild();
	ImGui::End();
}

static void HandlePropertiesContextWindow(PCKAssetFile& file, int propertyIndex = -1)
{
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Add"))
		{
			file.addProperty("KEY", u"VALUE");
		}
		if (propertyIndex > -1 && ImGui::MenuItem("Delete"))
		{
			file.removeProperty(propertyIndex);
		}
		ImGui::EndPopup();
	}
}

static void HandlePropertiesWindow(const PCKAssetFile& file)
{
	if (gLastPreviewedFile != &file) {
		gLastPreviewedFile = &file;
	}

	const auto properties = file.getProperties(); // make a copy of properties

	float propertyWindowPosX = ImGui::GetIO().DisplaySize.x * 0.25f;
	float propertyWindowHeight = (ImGui::GetIO().DisplaySize.y * 0.35f) - ImGui::GetFrameHeight();
	ImVec2 propertyWindowSize(ImGui::GetIO().DisplaySize.x * 0.75f, propertyWindowHeight);
	ImGui::SetNextWindowSize(propertyWindowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(propertyWindowPosX, ImGui::GetIO().DisplaySize.y - propertyWindowHeight), ImGuiCond_Always);

	ImGui::Begin("Properties", nullptr,
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_HorizontalScrollbar);

	// very stupid lol
	PCKAssetFile& editableFile = const_cast<PCKAssetFile&>(file);

	if (ImGui::BeginPopupContextWindow("PropertiesContextWindow"))
	{
		if (ImGui::MenuItem("Add"))
			editableFile.addProperty("KEY", u"VALUE");
		ImGui::EndPopup();
	}

	if (properties.empty()) {
		ImGui::Text("NO PROPERTIES");
	}
	else {
		int propertyIndex = 0;

		if (ImGui::BeginTable("PropertiesTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
		{
			float maxKeyWidth = ImGui::CalcTextSize("KEY").x;
			for (const auto& [key, _] : properties)
			{
				ImVec2 size = ImGui::CalcTextSize(key.c_str());
				if (size.x > maxKeyWidth)
					maxKeyWidth = size.x;
			}
			maxKeyWidth += 10.0f;

			ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, maxKeyWidth);
			ImGui::TableSetupColumn("Value");
			ImGui::TableHeadersRow();

			int propertyIndex = 0;
			for (const auto& [key, value] : properties)
			{
				ImGui::TableNextRow();

				char keyBuffer[0x11];
				char valueBuffer[0x1001];

				std::strncpy(keyBuffer, key.c_str(), sizeof(keyBuffer) - 1);
				keyBuffer[sizeof(keyBuffer) - 1] = '\0';

				std::string utf8Value = IO::ToUTF8(value);
				std::size_t len = std::min(utf8Value.size(), sizeof(valueBuffer) - 1);
				std::memcpy(valueBuffer, utf8Value.data(), len);
				valueBuffer[len] = '\0';

				bool modified = false;

				ImGui::TableSetColumnIndex(0);
				std::string keyLabel = "##Key" + std::to_string(propertyIndex);
				ImGui::SetNextItemWidth(-FLT_MIN); // this is needed to make the input the full size of the column for some reason
				if (ImGui::InputText(keyLabel.c_str(), keyBuffer, sizeof(keyBuffer)))
					modified = true;

				// context menu
				HandlePropertiesContextWindow(editableFile, propertyIndex);

				ImGui::TableSetColumnIndex(1);
				std::string valueLabel = "##Value" + std::to_string(propertyIndex);
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (ImGui::InputText(valueLabel.c_str(), valueBuffer, sizeof(valueBuffer)))
					modified = true;

				// context menu; again because I want it to work with both rows
				HandlePropertiesContextWindow(editableFile, propertyIndex);

				if (modified)
				{
					std::string keyText = keyBuffer;
					for (char& c : keyText)
						c = std::toupper(c);

					editableFile.setPropertyAtIndex(propertyIndex, keyText.empty() ? "KEY" : keyText, IO::ToUTF16(valueBuffer));
				}

				++propertyIndex;
			}

			ImGui::EndTable();
		}
	}

	ImGui::End();
}

static void HandlePCKNodeContextMenu(FileTreeNode& node)
{
	if (ImGui::BeginPopupContextItem()) {
		bool isFile = node.file;

		if (ImGui::BeginMenu("Extract")) {
			if (isFile && ImGui::MenuItem("File"))
			{
				ExtractFileDataDialog(*node.file);
			}
			if (!isFile && ImGui::MenuItem("Files"))
			{
				SaveFolderAsFiles(node);
			}
			if (!isFile && ImGui::MenuItem("Files with Properties"))
			{
				SaveFolderAsFiles(node, true);
			}

			bool hasProperties = node.file && !node.file->getProperties().empty();

			if (isFile && hasProperties && ImGui::MenuItem("Properties"))
			{
				SaveFilePropertiesDialog(*node.file);
			}

			if (isFile && hasProperties && ImGui::MenuItem("File with Properties"))
			{
				ExtractFileDataDialog(*node.file, true);
			}

			ImGui::EndMenu();
		}
		if (isFile && ImGui::BeginMenu("Replace"))
		{
			if (ImGui::MenuItem("File Data"))
			{
				if (SetDataFromFile(*node.file))
					ResetPreviewWindow();
			}
			if (ImGui::MenuItem("File Properties"))
			{
				SetFilePropertiesDialog(*node.file);
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Delete")) {
			if (ShowYesNoMessagePrompt("Are you sure?", "This is permanent and cannot be undone.\nIf this is a folder, all sub-files will be deleted too."))
			{
				DeleteNode(node, gTreeNodes);
				TreeToPCKFileCollection(gTreeNodes);
			}
			else
				ShowCancelledMessage();
		}
		ImGui::EndPopup();
	}
}

bool IsClicked()
{
	return (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) ||
		// for context support; selecting and opening a node when a context menu is already opened
		(ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered());
}

// Renders the passed node, also handles children or the node
static void RenderNode(FileTreeNode& node, std::vector<const FileTreeNode*>* visibleList = nullptr, bool shouldScroll = false, bool openFolder = false, bool closeFolder = false) {
	if (visibleList)
		visibleList->push_back(&node); // adds node to visible nodes vector, but only when needed

	bool isFolder = (node.file == nullptr);
	bool isSelected = (node.path == gSelectedNodePath);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	if (isSelected)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
		ScrollToNode(shouldScroll);
	}

	if (isFolder) {
		ImGui::PushID(node.path.c_str());
		ImGui::Image((void*)(intptr_t)gFolderIcon.id, ImVec2(48, 48));
		ImGui::SameLine();

		if (node.path == gSelectedNodePath && (openFolder || closeFolder))
			ImGui::SetNextItemOpen(openFolder, ImGuiCond_Always);

		bool open = ImGui::TreeNodeEx(node.path.c_str(), flags);

		if (IsClicked())
			gSelectedNodePath = node.path;

		HandlePCKNodeContextMenu(node);

		if (open) {
			for (auto& child : node.children)
				RenderNode(child, visibleList, shouldScroll, openFolder, closeFolder);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	else {
		const PCKAssetFile& file = *node.file;
		ImGui::Image((void*)(intptr_t)gFileIcons[file.getAssetType()].id, ImVec2(48, 48));
		ImGui::SameLine();
		if (ImGui::Selectable((std::filesystem::path(file.getPath()).filename().string() + "###" + file.getPath()).c_str(), isSelected))
			gSelectedNodePath = node.path;

		if (IsClicked())
			gSelectedNodePath = node.path;

		HandlePCKNodeContextMenu(node);
	}
}

// Renders the file tree... duh
static void RenderFileTree() {

	PCKFile* pckFile = gApp->CurrentPCKFile();

	if (!pckFile) return;

	bool shouldScroll = false;

	gVisibleNodes.clear();
	ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.25f, ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()));
	ImGui::Begin(std::string(pckFile->getFileName() + "###FileTree").c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	bool shouldOpenFolder = false;
	bool shouldCloseFolder = false;

	if (ImGui::IsWindowFocused() && !gSelectedNodePath.empty()) {
		shouldOpenFolder = ImGui::IsKeyPressed(ImGuiKey_RightArrow);
		shouldCloseFolder = !shouldOpenFolder && ImGui::IsKeyPressed(ImGuiKey_LeftArrow);
	}

	for (auto& node : gTreeNodes)
		RenderNode(node, &gVisibleNodes, shouldScroll, shouldOpenFolder, shouldCloseFolder);

	static int selectedIndex = -1;
	if (ImGui::IsWindowFocused() && !gVisibleNodes.empty()) {
		for (int i = 0; i < (int)gVisibleNodes.size(); ++i) {
			if (gVisibleNodes[i]->path == gSelectedNodePath) {
				selectedIndex = i;
				break;
			}
		}

		std::string previousPath = gSelectedNodePath;
		if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
			selectedIndex = std::max(0, selectedIndex - 1);
			gSelectedNodePath = gVisibleNodes[selectedIndex]->path;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
			selectedIndex = std::min((int)gVisibleNodes.size() - 1, selectedIndex + 1);
			gSelectedNodePath = gVisibleNodes[selectedIndex]->path;
		}
		if (gSelectedNodePath != previousPath)
			shouldScroll = true;
	}

	const PCKAssetFile* selectedFile = nullptr;
	for (const auto& _ : gTreeNodes) {

		FileTreeNode* selectedNode = FindNodeByPath(gSelectedNodePath, gTreeNodes);

		if (selectedNode && selectedNode->file)
			selectedFile = selectedNode->file;

		if (selectedFile) break;
	}
	if (selectedFile)
	{
		if (selectedFile->isImageType())
		{
			HandlePreviewWindow(*selectedFile);
		}

		HandlePropertiesWindow(*selectedFile);
	}

	shouldOpenFolder = false;
	shouldCloseFolder = false;
	ImGui::End();
}

void HandleFileTree() {
	BuildFileTree(gTreeNodes);
	RenderFileTree();
}