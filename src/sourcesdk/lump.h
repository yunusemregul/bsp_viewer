#define byte uint8_t

struct Vector
{
	float x;
	float y;
	float z;

	Vector(){};

	Vector(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	};

	Vector operator +(Vector b)
	{
		return Vector(
			x + b.x,
			y + b.y,
			z + b.z			
		);
	}

	Vector operator -(Vector b)
	{
		return Vector(
			x - b.x,
			y - b.y,
			z - b.z			
		);
	}

	Vector operator *(float i)
	{
		return Vector(
			x * i,
			y * i,
			z * i
		);
	}

	Vector operator *(Vector b)
	{
		return Vector(
			x * b.x,
			y * b.y,
			z * b.z
		);
	}

	Vector operator /(float i)
	{
		return Vector(
			x / i,
			y / i,
			z / i
		);		
	}

	Vector operator /(Vector b)
	{
		return Vector(
			x / b.x,
			y / b.y,
			z / b.z
		);
	}

	// normal multiplying
	Vector operator &(Vector b)
	{
		return Vector(
			x * (b.x==0 ? 1 : b.x),
			y * (b.y==0 ? 1 : b.y),
			z * (b.z==0 ? 1 : b.z)
		);
	}

	// for debugging purposes
	void tostring(char *str)
	{
		sprintf(str,"Vector(%.2f, %.2f, %.2f)", x, y, z);
	}
};

struct dplane_t
{
	Vector	normal;	// normal vector
	float	dist;	// distance from origin
	int	type;	// plane axis identifier
};

struct dedge_t
{
	unsigned short	v[2];	// vertex indices
};

struct dface_t
{
	unsigned short	planenum;		// the plane number
	byte		side;			// faces opposite to the node's plane direction
	byte		onNode;			// 1 of on node, 0 if in leaf
	int		firstedge;		// index into surfedges
	short		numedges;		// number of surfedges
	short		texinfo;		// texture info
	short		dispinfo;		// displacement info
	short		surfaceFogVolumeID;	// ?
	byte		styles[4];		// switchable lighting info
	int		lightofs;		// offset into lightmap lump
	float		area;			// face area in units^2
	int		LightmapTextureMinsInLuxels[2];	// texture lighting info
	int		LightmapTextureSizeInLuxels[2];	// texture lighting info
	int		origFace;		// original face this was split from
	unsigned short	numPrims;		// primitives
	unsigned short	firstPrimID;
	unsigned int	smoothingGroups;	// lightmap smoothing group
};

struct dmodel_t
{
	Vector	mins, maxs;		// bounding box
	Vector	origin;			// for sounds or lights
	int	headnode;		// index into node array
	int	firstface, numfaces;	// index into face array
};

struct texinfo_t
{
	float	textureVecs[2][4];	// [s/t][xyz offset]
	float	lightmapVecs[2][4];	// [s/t][xyz offset] - length is in units of texels/area
	int	flags;			// miptex flags	overrides
	int	texdata;		// Pointer to texture name, size, etc.
};

struct dtexdata_t
{
	Vector	reflectivity;		// RGB reflectivity
	int	nameStringTableID;	// index into TexdataStringTable
	int	width, height;		// source image
	int	view_width, view_height;
};

struct dDispVert
{
	Vector	vec;	// Vector field defining displacement volume.
	float	dist;	// Displacement distances.
	float	alpha;	// "per vertex" alpha values.
};

struct ddispinfo_t
{
	Vector			startPosition;		// start position used for orientation
	int			DispVertStart;		// Index into LUMP_DISP_VERTS.
	int			DispTriStart;		// Index into LUMP_DISP_TRIS.
	int			power;			// power - indicates size of surface (2^power	1)
	int			minTess;		// minimum tesselation allowed
	float			smoothingAngle;		// lighting smoothing angle
	int			contents;		// surface contents
	unsigned short		MapFace;		// Which map face this displacement comes from.
	int			LightmapAlphaStart;	// Index into ddisplightmapalpha.
	int			LightmapSamplePositionStart;	// Index into LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS.
	

	//CDispNeighbor		EdgeNeighbors[4];	// Indexed by NEIGHBOREDGE_ defines.
	//CDispCornerNeighbors	CornerNeighbors[4];	// Indexed by CORNER_ defines.

	// these neighbor infos are a bit complex classes
	// and we dont need them, so we replace them with corresponding
	// byte size, original size with them is 176 so
	// 176-sizeof(current ddispinfo_t) = 88
	// is their size
	char notused[88];
	
	unsigned int		AllowedVerts[10];	// active verticies
};