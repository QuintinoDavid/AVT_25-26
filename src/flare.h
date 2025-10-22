#ifndef FLARE_H
#define FLARE_H

#define NTEXTURES 5

typedef struct
{
	int textureId;
	float fDistance; // Distance along ray from source (0.0-1.0)
	float fSize;	 // Size scale
	float matDiffuse[4];
} FLARE_ELEMENT_DEF;

typedef struct
{
	float fScale;				   // Scale factor for flare elements
	float fMaxSize;				   // Max size of largest element (percentage of screen)
	int nPieces;				   // Number of elements
	FLARE_ELEMENT_DEF element[20]; // Increased from 10 to 20
} FLARE_DEF;

static const char *flareTextureNames[NTEXTURES] = {
	"crcl", "flar", "hxgn", "ring", "sun"};

void loadFlareFile(FLARE_DEF *flare, const char *filename);
unsigned int getTextureId(const char *name);

#endif