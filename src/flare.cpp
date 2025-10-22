#include "flare.h"
#include <stdio.h>
#include <string.h>

inline float clampf(const float x, const float min, const float max)
{
	return (x < min ? min : (x > max ? max : x));
}

unsigned int getTextureId(const char *name)
{
	for (int i = 0; i < NTEXTURES; ++i)
	{
		if (strncmp(name, flareTextureNames[i], strlen(name)) == 0)
			return i;
	}
	return -1;
}

void loadFlareFile(FLARE_DEF *flare, const char *filename)
{
	int n = 0;
	FILE *f;
	char buf[256];
	int fields;

	memset(flare, 0, sizeof(FLARE_DEF));

	f = fopen(filename, "r");
	if (f)
	{
		fgets(buf, sizeof(buf), f);
		sscanf(buf, "%f %f", &flare->fScale, &flare->fMaxSize);

		while (!feof(f) && n < 20) // Add bounds check
		{
			char name[8] = {
				'\0',
			};
			double dDist = 0.0, dSize = 0.0;
			float color[4];
			int id;

			fgets(buf, sizeof(buf), f);
			fields = sscanf(buf, "%4s %lf %lf ( %f %f %f %f )",
							name, &dDist, &dSize, &color[3], &color[0], &color[1], &color[2]);
			if (fields == 7)
			{
				for (int i = 0; i < 4; ++i)
					color[i] = clampf(color[i] / 255.0f, 0.0f, 1.0f);

				id = getTextureId(name);
				if (id < 0)
				{
					printf("Texture name not recognized: %s\n", name);
					continue; // Skip this element instead of storing garbage
				}

				flare->element[n].textureId = id;
				flare->element[n].fDistance = (float)dDist;
				flare->element[n].fSize = (float)dSize;
				memcpy(flare->element[n].matDiffuse, color, 4 * sizeof(float));
				++n;
			}
		}

		flare->nPieces = n;
		// printf("Loaded %d flare elements from %s\n", n, filename);
		fclose(f);
	}
	else
		printf("Flare file opening error\n");
}