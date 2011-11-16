/*
Minetest-c55
Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "nodedef.h"

#include "main.h" // For g_settings
#include "nodemetadata.h"
#ifndef SERVER
#include "tile.h"
#endif
#include "log.h"
#include "settings.h"
#include "nameidmapping.h"

/*
	NodeBox
*/

void NodeBox::serialize(std::ostream &os) const
{
	writeU8(os, 0); // version
	writeU8(os, type);
	writeV3F1000(os, fixed.MinEdge);
	writeV3F1000(os, fixed.MaxEdge);
	writeV3F1000(os, wall_top.MinEdge);
	writeV3F1000(os, wall_top.MaxEdge);
	writeV3F1000(os, wall_bottom.MinEdge);
	writeV3F1000(os, wall_bottom.MaxEdge);
	writeV3F1000(os, wall_side.MinEdge);
	writeV3F1000(os, wall_side.MaxEdge);
}

void NodeBox::deSerialize(std::istream &is)
{
	int version = readU8(is);
	if(version != 0)
		throw SerializationError("unsupported NodeBox version");
	type = (enum NodeBoxType)readU8(is);
	fixed.MinEdge = readV3F1000(is);
	fixed.MaxEdge = readV3F1000(is);
	wall_top.MinEdge = readV3F1000(is);
	wall_top.MaxEdge = readV3F1000(is);
	wall_bottom.MinEdge = readV3F1000(is);
	wall_bottom.MaxEdge = readV3F1000(is);
	wall_side.MinEdge = readV3F1000(is);
	wall_side.MaxEdge = readV3F1000(is);
}

/*
	MaterialSpec
*/

void MaterialSpec::serialize(std::ostream &os) const
{
	os<<serializeString(tname);
	writeU8(os, backface_culling);
}

void MaterialSpec::deSerialize(std::istream &is)
{
	tname = deSerializeString(is);
	backface_culling = readU8(is);
}

/*
	ContentFeatures
*/

ContentFeatures::ContentFeatures()
{
	reset();
}

ContentFeatures::~ContentFeatures()
{
	delete initial_metadata;
#ifndef SERVER
	for(u16 j=0; j<CF_SPECIAL_COUNT; j++){
		delete special_materials[j];
		delete special_aps[j];
	}
#endif
}

void ContentFeatures::reset()
{
	/*
		Cached stuff
	*/
#ifndef SERVER
	inventory_texture = NULL;
	
	for(u16 j=0; j<CF_SPECIAL_COUNT; j++){
		special_materials[j] = NULL;
		special_aps[j] = NULL;
	}
	solidness = 2;
	visual_solidness = 0;
	backface_culling = true;
#endif
	used_texturenames.clear();
	/*
		Actual data
	*/
	name = "";
	drawtype = NDT_NORMAL;
	visual_scale = 1.0;
	for(u32 i=0; i<6; i++)
		tname_tiles[i] = "";
	for(u16 j=0; j<CF_SPECIAL_COUNT; j++)
		mspec_special[j] = MaterialSpec();
	tname_inventory = "";
	alpha = 255;
	post_effect_color = video::SColor(0, 0, 0, 0);
	param_type = CPT_NONE;
	is_ground_content = false;
	light_propagates = false;
	sunlight_propagates = false;
	walkable = true;
	pointable = true;
	diggable = true;
	climbable = false;
	buildable_to = false;
	wall_mounted = false;
	air_equivalent = false;
	often_contains_mineral = false;
	dug_item = "";
	extra_dug_item = "";
	extra_dug_item_rarity = 2;
	initial_metadata = NULL;
	liquid_type = LIQUID_NONE;
	liquid_alternative_flowing = CONTENT_IGNORE;
	liquid_alternative_source = CONTENT_IGNORE;
	liquid_viscosity = 0;
	light_source = 0;
	damage_per_second = 0;
	selection_box = NodeBox();
	material = MaterialProperties();
	cookresult_item = ""; // Cannot be cooked
	furnace_cooktime = 3.0;
	furnace_burntime = -1.0; // Cannot be burned
}

void ContentFeatures::serialize(std::ostream &os)
{
	writeU8(os, 0); // version
	os<<serializeString(name);
	writeU8(os, drawtype);
	writeF1000(os, visual_scale);
	writeU8(os, 6);
	for(u32 i=0; i<6; i++)
		os<<serializeString(tname_tiles[i]);
	os<<serializeString(tname_inventory);
	writeU8(os, CF_SPECIAL_COUNT);
	for(u32 i=0; i<CF_SPECIAL_COUNT; i++){
		mspec_special[i].serialize(os);
	}
	writeU8(os, alpha);
	writeU8(os, post_effect_color.getAlpha());
	writeU8(os, post_effect_color.getRed());
	writeU8(os, post_effect_color.getGreen());
	writeU8(os, post_effect_color.getBlue());
	writeU8(os, param_type);
	writeU8(os, is_ground_content);
	writeU8(os, light_propagates);
	writeU8(os, sunlight_propagates);
	writeU8(os, walkable);
	writeU8(os, pointable);
	writeU8(os, diggable);
	writeU8(os, climbable);
	writeU8(os, buildable_to);
	writeU8(os, wall_mounted);
	writeU8(os, air_equivalent);
	writeU8(os, often_contains_mineral);
	os<<serializeString(dug_item);
	os<<serializeString(extra_dug_item);
	writeS32(os, extra_dug_item_rarity);
	if(initial_metadata){
		writeU8(os, true);
		initial_metadata->serialize(os);
	} else {
		writeU8(os, false);
	}
	writeU8(os, liquid_type);
	writeU16(os, liquid_alternative_flowing);
	writeU16(os, liquid_alternative_source);
	writeU8(os, liquid_viscosity);
	writeU8(os, light_source);
	writeU32(os, damage_per_second);
	selection_box.serialize(os);
	material.serialize(os);
	os<<serializeString(cookresult_item);
	writeF1000(os, furnace_cooktime);
	writeF1000(os, furnace_burntime);
}

void ContentFeatures::deSerialize(std::istream &is, IGameDef *gamedef)
{
	int version = readU8(is);
	if(version != 0)
		throw SerializationError("unsupported ContentFeatures version");
	name = deSerializeString(is);
	drawtype = (enum NodeDrawType)readU8(is);
	visual_scale = readF1000(is);
	if(readU8(is) != 6)
		throw SerializationError("unsupported tile count");
	for(u32 i=0; i<6; i++)
		tname_tiles[i] = deSerializeString(is);
	tname_inventory = deSerializeString(is);
	if(readU8(is) != CF_SPECIAL_COUNT)
		throw SerializationError("unsupported CF_SPECIAL_COUNT");
	for(u32 i=0; i<CF_SPECIAL_COUNT; i++){
		mspec_special[i].deSerialize(is);
	}
	alpha = readU8(is);
	post_effect_color.setAlpha(readU8(is));
	post_effect_color.setRed(readU8(is));
	post_effect_color.setGreen(readU8(is));
	post_effect_color.setBlue(readU8(is));
	param_type = (enum ContentParamType)readU8(is);
	is_ground_content = readU8(is);
	light_propagates = readU8(is);
	sunlight_propagates = readU8(is);
	walkable = readU8(is);
	pointable = readU8(is);
	diggable = readU8(is);
	climbable = readU8(is);
	buildable_to = readU8(is);
	wall_mounted = readU8(is);
	air_equivalent = readU8(is);
	often_contains_mineral = readU8(is);
	dug_item = deSerializeString(is);
	extra_dug_item = deSerializeString(is);
	extra_dug_item_rarity = readS32(is);
	if(readU8(is)){
		initial_metadata = NodeMetadata::deSerialize(is, gamedef);
	} else {
		initial_metadata = NULL;
	}
	liquid_type = (enum LiquidType)readU8(is);
	liquid_alternative_flowing = readU16(is);
	liquid_alternative_source = readU16(is);
	liquid_viscosity = readU8(is);
	light_source = readU8(is);
	damage_per_second = readU32(is);
	selection_box.deSerialize(is);
	material.deSerialize(is);
	cookresult_item = deSerializeString(is);
	furnace_cooktime = readF1000(is);
	furnace_burntime = readF1000(is);
}

void ContentFeatures::setTexture(u16 i, std::string name)
{
	used_texturenames.insert(name);
	tname_tiles[i] = name;
	if(tname_inventory == "")
		tname_inventory = name;
}

void ContentFeatures::setAllTextures(std::string name)
{
	for(u16 i=0; i<6; i++)
		setTexture(i, name);
	// Force inventory texture too
	setInventoryTexture(name);
}

void ContentFeatures::setSpecialMaterial(u16 i, const MaterialSpec &mspec)
{
	assert(i < CF_SPECIAL_COUNT);
	mspec_special[i] = mspec;
}

void ContentFeatures::setInventoryTexture(std::string imgname)
{
	tname_inventory = imgname + "^[forcesingle";
}

void ContentFeatures::setInventoryTextureCube(std::string top,
		std::string left, std::string right)
{
	str_replace_char(top, '^', '&');
	str_replace_char(left, '^', '&');
	str_replace_char(right, '^', '&');

	std::string imgname_full;
	imgname_full += "[inventorycube{";
	imgname_full += top;
	imgname_full += "{";
	imgname_full += left;
	imgname_full += "{";
	imgname_full += right;
	tname_inventory = imgname_full;
}

/*
	CNodeDefManager
*/

class CNodeDefManager: public IWritableNodeDefManager
{
public:
	void clear()
	{
		m_name_id_mapping.clear();
		for(u16 i=0; i<=MAX_CONTENT; i++)
		{
			ContentFeatures &f = m_content_features[i];
			f.reset(); // Reset to defaults
			f.setAllTextures("unknown_block.png");
		}
		
		// Set CONTENT_AIR
		{
			ContentFeatures f;
			f.name = "air";
			f.drawtype = NDT_AIRLIKE;
			f.param_type = CPT_LIGHT;
			f.light_propagates = true;
			f.sunlight_propagates = true;
			f.walkable = false;
			f.pointable = false;
			f.diggable = false;
			f.buildable_to = true;
			f.air_equivalent = true;
			set(CONTENT_AIR, f);
		}
		// Set CONTENT_IGNORE
		{
			ContentFeatures f;
			f.name = "ignore";
			f.drawtype = NDT_AIRLIKE;
			f.param_type = CPT_LIGHT;
			f.light_propagates = true;
			f.sunlight_propagates = true;
			f.walkable = false;
			f.pointable = false;
			f.diggable = false;
			f.buildable_to = true;
			f.air_equivalent = true;
			set(CONTENT_IGNORE, f);
		}
	}
	// CONTENT_IGNORE = not found
	content_t getFreeId(bool require_full_param2)
	{
		// If allowed, first search in the large 4-byte-param2 pool
		if(!require_full_param2){
			for(u16 i=0x800; i<=0xfff; i++){
				const ContentFeatures &f = m_content_features[i];
				if(f.name == "")
					return i;
			}
		}
		// Then search from the small 8-byte-param2 pool
		for(u16 i=0; i<=125; i++){
			const ContentFeatures &f = m_content_features[i];
			if(f.name == "")
				return i;
		}
		return CONTENT_IGNORE;
	}
	CNodeDefManager()
	{
		clear();
	}
	virtual ~CNodeDefManager()
	{
	}
	virtual IWritableNodeDefManager* clone()
	{
		CNodeDefManager *mgr = new CNodeDefManager();
		for(u16 i=0; i<=MAX_CONTENT; i++)
		{
			mgr->set(i, get(i));
		}
		return mgr;
	}
	virtual const ContentFeatures& get(content_t c) const
	{
		assert(c <= MAX_CONTENT);
		return m_content_features[c];
	}
	virtual const ContentFeatures& get(const MapNode &n) const
	{
		return get(n.getContent());
	}
	virtual bool getId(const std::string &name, content_t &result) const
	{
		return m_name_id_mapping.getId(name, result);
	}
	virtual const ContentFeatures& get(const std::string &name) const
	{
		content_t id = CONTENT_IGNORE;
		getId(name, id);
		return get(id);
	}
	// IWritableNodeDefManager
	virtual void set(content_t c, const ContentFeatures &def)
	{
		infostream<<"registerNode: registering content id \""<<c
				<<"\": name=\""<<def.name<<"\""<<std::endl;
		assert(c <= MAX_CONTENT);
		m_content_features[c] = def;
		if(def.name != "")
			m_name_id_mapping.set(c, def.name);
	}
	virtual content_t set(const std::string &name,
			const ContentFeatures &def)
	{
		assert(name == def.name);
		u16 id = CONTENT_IGNORE;
		bool found = m_name_id_mapping.getId(name, id);
		if(!found){
			// Determine if full param2 is required
			bool require_full_param2 = (
				def.liquid_type == LIQUID_FLOWING
				||
				def.drawtype == NDT_FLOWINGLIQUID
				||
				def.drawtype == NDT_TORCHLIKE
				||
				def.drawtype == NDT_SIGNLIKE
			);
			// Get some id
			id = getFreeId(require_full_param2);
			if(id == CONTENT_IGNORE)
				return CONTENT_IGNORE;
			if(name != "")
				m_name_id_mapping.set(id, name);
		}
		set(id, def);
		return id;
	}
	virtual content_t allocateDummy(const std::string &name)
	{
		assert(name != "");
		ContentFeatures f;
		f.name = name;
		f.setAllTextures("unknown_block.png");
		return set(name, f);
	}
	virtual void updateTextures(ITextureSource *tsrc)
	{
#ifndef SERVER
		infostream<<"CNodeDefManager::updateTextures(): Updating "
				<<"textures in node definitions"<<std::endl;

		bool new_style_water = g_settings->getBool("new_style_water");
		bool new_style_leaves = g_settings->getBool("new_style_leaves");
		bool opaque_water = g_settings->getBool("opaque_water");
		
		for(u16 i=0; i<=MAX_CONTENT; i++)
		{
			ContentFeatures *f = &m_content_features[i];

			switch(f->drawtype){
			default:
			case NDT_NORMAL:
				f->solidness = 2;
				break;
			case NDT_AIRLIKE:
				f->solidness = 0;
				break;
			case NDT_LIQUID:
				assert(f->liquid_type == LIQUID_SOURCE);
				if(opaque_water)
					f->alpha = 255;
				if(new_style_water){
					f->solidness = 0;
				} else {
					f->solidness = 1;
					if(f->alpha == 255)
						f->solidness = 2;
				}
				break;
			case NDT_FLOWINGLIQUID:
				assert(f->liquid_type == LIQUID_FLOWING);
				f->solidness = 0;
				if(opaque_water)
					f->alpha = 255;
				break;
			case NDT_GLASSLIKE:
				f->solidness = 0;
				f->visual_solidness = 1;
				break;
			case NDT_ALLFACES:
				f->solidness = 0;
				f->visual_solidness = 1;
				break;
			case NDT_ALLFACES_OPTIONAL:
				if(new_style_leaves){
					f->drawtype = NDT_ALLFACES;
					f->solidness = 0;
					f->visual_solidness = 1;
				} else {
					f->drawtype = NDT_NORMAL;
					f->solidness = 1;
					for(u32 i=0; i<6; i++){
						f->tname_tiles[i] = std::string("[noalpha:")
								+ f->tname_tiles[i];
					}
				}
				break;
			case NDT_TORCHLIKE:
			case NDT_SIGNLIKE:
			case NDT_PLANTLIKE:
			case NDT_FENCELIKE:
			case NDT_RAILLIKE:
				f->solidness = 0;
				break;
			}

			// Inventory texture
			if(f->tname_inventory != "")
				f->inventory_texture = tsrc->getTextureRaw(f->tname_inventory);
			else
				f->inventory_texture = NULL;
			// Tile textures
			for(u16 j=0; j<6; j++){
				if(f->tname_tiles[j] == "")
					continue;
				f->tiles[j].texture = tsrc->getTexture(f->tname_tiles[j]);
				f->tiles[j].alpha = f->alpha;
				if(f->alpha == 255)
					f->tiles[j].material_type = MATERIAL_ALPHA_SIMPLE;
				else
					f->tiles[j].material_type = MATERIAL_ALPHA_VERTEX;
				if(f->backface_culling)
					f->tiles[j].material_flags |= MATERIAL_FLAG_BACKFACE_CULLING;
				else
					f->tiles[j].material_flags &= ~MATERIAL_FLAG_BACKFACE_CULLING;
			}
			// Special textures
			for(u16 j=0; j<CF_SPECIAL_COUNT; j++){
				// Remove all stuff
				if(f->special_aps[j]){
					delete f->special_aps[j];
					f->special_aps[j] = NULL;
				}
				if(f->special_materials[j]){
					delete f->special_materials[j];
					f->special_materials[j] = NULL;
				}
				// Skip if should not exist
				if(f->mspec_special[j].tname == "")
					continue;
				// Create all stuff
				f->special_aps[j] = new AtlasPointer(
						tsrc->getTexture(f->mspec_special[j].tname));
				f->special_materials[j] = new video::SMaterial;
				f->special_materials[j]->setFlag(video::EMF_LIGHTING, false);
				f->special_materials[j]->setFlag(video::EMF_BACK_FACE_CULLING,
						f->mspec_special[j].backface_culling);
				f->special_materials[j]->setFlag(video::EMF_BILINEAR_FILTER, false);
				f->special_materials[j]->setFlag(video::EMF_FOG_ENABLE, true);
				f->special_materials[j]->setTexture(0, f->special_aps[j]->atlas);
				if(f->alpha != 255)
					f->special_materials[j]->MaterialType =
							video::EMT_TRANSPARENT_VERTEX_ALPHA;
			}
		}
#endif
	}
	void serialize(std::ostream &os)
	{
		u16 count = 0;
		std::ostringstream tmp_os(std::ios::binary);
		for(u16 i=0; i<=MAX_CONTENT; i++)
		{
			if(i == CONTENT_IGNORE || i == CONTENT_AIR)
				continue;
			ContentFeatures *f = &m_content_features[i];
			if(f->name == "")
				continue;
			writeU16(tmp_os, i);
			f->serialize(tmp_os);
			count++;
		}
		writeU16(os, count);
		os<<serializeLongString(tmp_os.str());
	}
	void deSerialize(std::istream &is, IGameDef *gamedef)
	{
		clear();
		u16 count = readU16(is);
		std::istringstream tmp_is(deSerializeLongString(is), std::ios::binary);
		for(u16 n=0; n<count; n++){
			u16 i = readU16(tmp_is);
			if(i > MAX_CONTENT){
				errorstream<<"ContentFeatures::deSerialize(): "
						<<"Too large content id: "<<i<<std::endl;
				continue;
			}
			ContentFeatures *f = &m_content_features[i];
			f->deSerialize(tmp_is, gamedef);
			if(f->name != "")
				m_name_id_mapping.set(i, f->name);
		}
	}
private:
	ContentFeatures m_content_features[MAX_CONTENT+1];
	NameIdMapping m_name_id_mapping;
};

IWritableNodeDefManager* createNodeDefManager()
{
	return new CNodeDefManager();
}
