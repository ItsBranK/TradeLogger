#include "GuidWrapper.hpp"

GuidWrapper::GuidWrapper() : A(0), B(0), C(0), D(0) {}

GuidWrapper::GuidWrapper(int32_t abcd) : A(abcd), B(abcd), C(abcd), D(abcd) {}

GuidWrapper::GuidWrapper(int32_t a, int32_t b, int32_t c, int32_t d) : A(a), B(b), C(c), D(d) {}

GuidWrapper::GuidWrapper(const GuidWrapper& guid) : A(guid.A), B(guid.B), C(guid.C), D(guid.D) {}

GuidWrapper::~GuidWrapper() {}

void GuidWrapper::Invalidate()
{
	A = 0;
	B = 0;
	C = 0;
	D = 0;
}

bool GuidWrapper::IsValid() const
{
	return ((A | B | C | D) != 0);
}

std::string GuidWrapper::ToString() const
{
	return ToString(EGuidFormats::Digits);
}

std::string GuidWrapper::ToString(EGuidFormats format) const
{
	char buffer[128];

	switch (format)
	{
	case EGuidFormats::DigitsWithHyphens:
		sprintf_s(buffer, "%08X-%04X-%04X-%04X-%04X%08X", A, B >> 16, B & 0xFFFF, C >> 16, C & 0xFFFF, D);
		return buffer;
	case EGuidFormats::DigitsWithHyphensInBraces:
		sprintf_s(buffer, "{%08X-%04X-%04X-%04X-%04X%08X}", A, B >> 16, B & 0xFFFF, C >> 16, C & 0xFFFF, D);
		return buffer;
	case EGuidFormats::DigitsWithHyphensInParentheses:
		sprintf_s(buffer, "(%08X-%04X-%04X-%04X-%04X%08X)", A, B >> 16, B & 0xFFFF, C >> 16, C & 0xFFFF, D);
		return buffer;
	case EGuidFormats::HexValuesInBraces:
		sprintf_s(buffer, "{0x%08X,0x%04X,0x%04X,{0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}}", A, B >> 16, B & 0xFFFF, C >> 24, (C >> 16) & 0xFF, (C >> 8) & 0xFF, C & 0XFF, D >> 24, (D >> 16) & 0XFF, (D >> 8) & 0XFF, D & 0XFF);
		return buffer;
	case EGuidFormats::UniqueObjectGuid:
		sprintf_s(buffer, "%08X-%08X-%08X-%08X", A, B, C, D);
		return buffer;
	default:
		sprintf_s(buffer, "%08X%08X%08X%08X", A, B, C, D);
		return buffer;
	}
}

GuidWrapper& GuidWrapper::operator=(const GuidWrapper& other)
{
	A = other.A;
	B = other.B;
	C = other.C;
	D = other.D;
	return *this;
}