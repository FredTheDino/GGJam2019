#ifndef __BLOCK_ID__
#define __BLOCK_ID__
// This is a generic id that are used by the 
// "pile" data structure. Where things
// can be replaced and created efficently.
struct BlockID
{
	union 
	{
		s32 pos; // The position in the list.
		s32 next_free; // For when it's dead.
	};
	s32 uid; // Unique identifier.

	bool operator== (BlockID &that) const
	{
		return this->pos == that.pos && this->uid == that.uid;
	}
};

// NOTE: These maybe should be moved to
// their respective files.
struct Asset;
struct AssetID : public BlockID
{
		Asset operator* ();
};;

struct Entity;
struct EntityID : public BlockID
{
	Entity *operator-> ();
	operator bool();
};

struct Body;
struct BodyID : public BlockID 
{
	Body *operator-> ();
	operator bool();
};

typedef BlockID ShapeID;
#endif
