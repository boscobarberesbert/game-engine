#ifndef __I_TEXTURE_H__
#define __I_TEXTURE_H__

class R_Texture;

class I_Texture
{
public:
	I_Texture();
	~I_Texture();

	bool Import(const char* path, R_Texture* texture);
	bool Save(const R_Texture* texture, const char* path);
};

#endif // !__I_TEXTURE_H__