#include <ndspy.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	FILE *file;
	int channels;
	int width, height;
} *MyImageType;
//---------------------------------------------------------------
//DspyImageOpen
//---------------------------------------------------------------
PtDspyError DspyImageOpen(PtDspyImageHandle *pvImage,
	const char *drivername,
	const char *filename,
	int width,
	int height,
	int paramCount,
	const UserParameter *parameters,
	int formatCount,
	PtDspyDevFormat *format,
	PtFlagStuff *flagstuff)
{
	PtDspyError ret;
	MyImageType image;


	/* We want to receive the pixels one after the other */

	flagstuff->flags |= PkDspyFlagsWantsScanLineOrder;
	/* Do stupidity checking */

	if (width == 0) width = 640;
	if (height == 0) height = 480;

	image = NULL;
	ret = PkDspyErrorNone;

	image = (MyImageType)malloc(sizeof(*image));

	if (NULL != image)
	{
		int i;

		image->channels = formatCount;
		image->width = width;
		image->height = height;
		image->file = fopen(filename, "wb");
		if (image->file)
		{
			fwrite(&formatCount, sizeof(formatCount), 1, image->file);
			for (i = 0; i < formatCount; i++)
			{
				format[i].type = PkDspyFloat32;
				fwrite(format[i].name, strlen(format[i].name) + 1, 1, image->file);
			}
		}
		else
		{
			free(image);
			image = NULL;
			ret = PkDspyErrorNoResource;
		}
	}
	else
		ret = PkDspyErrorNoMemory;

	*pvImage = image;
	return ret;
}

//---------------------------------------------------------------
//DspyImageQuery
//---------------------------------------------------------------
PtDspyError DspyImageQuery(PtDspyImageHandle pvImage,
	PtDspyQueryType querytype,
	int datalen,
	void *data)
{
	PtDspyError ret;
	MyImageType image = (MyImageType)pvImage;

	ret = PkDspyErrorNone;

	if (datalen > 0 && NULL != data)
	{
		switch (querytype)
		{
		case PkOverwriteQuery:
		{
			PtDspyOverwriteInfo overwriteInfo;

			if (datalen > sizeof(overwriteInfo))
				datalen = sizeof(overwriteInfo);
			overwriteInfo.overwrite = 1;
			overwriteInfo.interactive = 0;
			memcpy(data, &overwriteInfo, datalen);
			break;
		}
		case PkSizeQuery:
		{
			PtDspySizeInfo sizeInfo;

			if (datalen > sizeof(sizeInfo)) {
				datalen = sizeof(sizeInfo);
			}
			if (image)
			{
				if (0 == image->width ||
					0 == image->height)
				{
					image->width = 640;
					image->height = 480;
				}
				sizeInfo.width = image->width;
				sizeInfo.height = image->height;
				sizeInfo.aspectRatio = 1.0f;
			}
			else
			{
				sizeInfo.width = 640;
				sizeInfo.height = 480;
				sizeInfo.aspectRatio = 1.0f;
			}
			memcpy(data, &sizeInfo, datalen);
			break;
		}
		case PkRenderingStartQuery:
		{
			PtDspyRenderingStartQuery startLocation;

			if (datalen > sizeof(startLocation))
				datalen = sizeof(startLocation);

			if (image) {
				// initialize values in startLocation
				memcpy(data, &startLocation, datalen);
			}
			else {
				ret = PkDspyErrorUndefined;
			}
			break;
		}

		default:
			ret = PkDspyErrorUnsupported;
			break;
		}
	}
	else
	{
		ret = PkDspyErrorBadParams;
	}
	return ret;
}
//---------------------------------------------------------------
//DspyImageData
//---------------------------------------------------------------
PtDspyError DspyImageData(PtDspyImageHandle pvImage,
	int xmin,
	int xmax_plusone,
	int ymin,
	int ymax_plusone,
	int entrysize,
	const unsigned char *data)
{
	PtDspyError ret;
	MyImageType image = (MyImageType)pvImage;
	int oldx;

	oldx = xmin;
	for (; ymin < ymax_plusone; ymin++)
	{
		for (xmin = oldx; xmin < xmax_plusone; xmin++)
		{
			fwrite(data, sizeof(float), image->channels, image->file);
			data += entrysize;
		}
	}
	ret = PkDspyErrorNone;
	return ret;
}
//---------------------------------------------------------------
//DspyImageClose
//---------------------------------------------------------------
PtDspyError DspyImageClose(PtDspyImageHandle pvImage)
{
	PtDspyError ret;
	MyImageType image = (MyImageType)pvImage;

	fclose(image->file);
	free(image);
	ret = PkDspyErrorNone;
	return ret;
}
