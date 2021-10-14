#pragma once
#include <string>

enum class EGuidFormats : uint8_t
{
	Digits =							0, // Example: 00000000000000000000000000000000
	DigitsWithHyphens =					1, // Example: 00000000-0000-0000-0000-000000000000
	DigitsWithHyphensInBraces =			2, // Example: {00000000-0000-0000-0000-000000000000}
	DigitsWithHyphensInParentheses =	3, // Example: (00000000-0000-0000-0000-000000000000)
	HexValuesInBraces =					4, // Example: {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
	UniqueObjectGuid =					5  // Example: 00000000-00000000-00000000-00000000
};

class GuidWrapper
{
public:
	int32_t A, B, C, D;

public:
	GuidWrapper();
	GuidWrapper(const GuidWrapper& guid);
	GuidWrapper(int32_t abcd);
	GuidWrapper(int32_t a, int32_t b, int32_t c, int32_t d);
	~GuidWrapper();

public:
	void Invalidate();
	bool IsValid() const;
	std::string ToString() const;
	std::string ToString(EGuidFormats format) const;

public:
	GuidWrapper operator=(const GuidWrapper& other);
};