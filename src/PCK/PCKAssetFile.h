#pragma once
#include <vector>
#include <string>
#include <fstream>

// PCK Asset File and Asset File Types research done by NessieHax/Miku666/nullptr, myself (May/MattNL), and many others over the years.

// Files used inside of PCK Files
class PCKAssetFile
{
public:
	enum class Type
	{
		// *.png for Skins; used for Skin initialization
		SKIN,
		// *.png for Capes; used for Cape initialization
		CAPE,
		// *.png for generic textures and resources
		TEXTURE,
		// Unused; likely a PCK file used for UI sprites at some point in development
		UI_DATA,
		// "0" file for Pack Initialization
		INFO,
		// .pck data file for Texture Pack Initialization
		TEXTURE_PACK_INFO,
		// languages.loc/localisation.loc for string localization
		LOCALISATION,
		// GameRules.grf compressed file for World Template initialization
		GAME_RULES,
		// .pck data file for Custom Texture Pack Music Cues
		AUDIO_DATA,
		// colours.col file for Texture Pack Custom Colo(u)r Palettes
		COLOUR_TABLE,
		// GameRules.grh non compressed file for World Template initialization
		GAME_RULES_HEADER,
		// .pck data file for Skin initalization and extended custom skin features
		SKIN_DATA,
		// models.bin file for Custom Entity Models for Texture Packs
		MODELS,
		// behaviours.bin file for... only changing the position of where a player is positioned when sitting lol
		BEHAVIOURS,
		// entityMaterials.bin file for changing the rendering properties of certain Entity textures for Texture Packs
		MATERIALS,
		// Just a total value for loops
		PCK_ASSET_TYPES_TOTAL
	};

	// Gets a asset type as a string
	constexpr const char* PCKAssetFile::getAssetTypeString() const
	{
		return getAssetTypeString(mAssetType);
	}

	// Gets a asset type as a string
	static constexpr const char* PCKAssetFile::getAssetTypeString(PCKAssetFile::Type type)
	{
		switch (type)
		{
		case Type::SKIN: return "SKIN";
		case Type::CAPE: return "CAPE";
		case Type::TEXTURE: return "TEXTURE";
		case Type::UI_DATA: return "UI_DATA";
		case Type::INFO: return "INFO";
		case Type::TEXTURE_PACK_INFO: return "TEXTURE_PACK_INFO";
		case Type::LOCALISATION: return "LOCALISATION";
		case Type::GAME_RULES: return "GAME_RULES";
		case Type::AUDIO_DATA: return "AUDIO_DATA";
		case Type::COLOUR_TABLE: return "COLOUR_TABLE";
		case Type::GAME_RULES_HEADER: return "GAME_RULES_HEADER";
		case Type::SKIN_DATA: return "SKIN_DATA";
		case Type::MODELS: return "MODELS";
		case Type::BEHAVIOURS: return "BEHAVIOURS";
		case Type::MATERIALS: return "MATERIALS";
		default: return "UNKNOWN";
		}
	}

	PCKAssetFile(const std::string& path, const std::vector<unsigned char>& data, Type assetType)
		: mAssetType(assetType), mData(data), mPath(path) {}

	~PCKAssetFile() {
		mData.clear();
		mPath.clear();
	}

	// Gets the file size, in bytes
	std::size_t getFileSize() const;

	// Gets the file data as a const unsigned char vector
	const std::vector<unsigned char>& getData() const;

	// Sets the file data with a const unsigned char vector
	void setData(const std::vector<unsigned char>& data);

	// Gets the file path
	const std::string& getPath() const;

	// Sets the file path
	void setPath(const std::string& inpath);

	// Get the asset type
	Type getAssetType() const;

	// Checks if the file is a Skin, Cape or Texture Asset Type
	bool isImageType() const;

	// Adds a property to the file
	void addProperty(const std::string& key, const std::string& value);

private:
	Type mAssetType{ Type::SKIN };
	std::vector<unsigned char> mData;
	std::string mPath;
	std::vector<std::pair<std::string, std::string>> mProperties;
};