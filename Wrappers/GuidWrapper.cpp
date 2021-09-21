#include "GuidWrapper.hpp"

GuidWrapper::GuidWrapper() : A(0), B(0), C(0), D(0) { }

GuidWrapper::GuidWrapper(const GuidWrapper& guid) : A(guid.A), B(guid.B), C(guid.C), D(guid.D) { }

GuidWrapper::GuidWrapper(int32_t a, int32_t b, int32_t c, int32_t d) : A(a), B(b), C(c), D(d) { }

GuidWrapper::~GuidWrapper() { }

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
	//case EGuidFormats::Short:
	//{
	//	const uint32_t Data[] = { A,B,C,D };
	//	FString Result = FBase64::Encode(reinterpret_cast<const uint8*>(&Data), sizeof(Data));

	//	Result.ReplaceCharInline(TEXT('+'), TEXT('-'), ESearchCase::CaseSensitive);
	//	Result.ReplaceCharInline(TEXT('/'), TEXT('_'), ESearchCase::CaseSensitive);

	//	// Remove trailing '=' base64 padding
	//	check(Result.Len() == 24);
	//	Result.RemoveAt(22, 2, false);

	//	return Result;
	//}

	//case EGuidFormats::Base36Encoded:
	//{
	//	static const uint8_t Alphabet[36] =
	//	{
	//		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	//		'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
	//		'W', 'X', 'Y', 'Z'
	//	};

	//	FUInt128 Value(A, B, C, D);

	//	FString Result;
	//	Result.Reserve(26);
	//	TArray<TCHAR>& OutCharArray = Result.GetCharArray();

	//	while (Value > 0)
	//	{
	//		uint32_t Remainder;
	//		Value = Value.Divide(36, Remainder);
	//		OutCharArray.Add(Alphabet[Remainder]);
	//	}

	//	for (int32_t i = OutCharArray.Num(); i < 25; i++)
	//	{
	//		OutCharArray.Add('0');
	//	}

	//	OutCharArray.Add(0);

	//	check(Result.Len() == 25);
	//	return Result.Reverse();
	//}

	default:
		sprintf_s(buffer, "%08X%08X%08X%08X", A, B, C, D);
		return buffer;
	}
}

GuidWrapper GuidWrapper::operator=(const GuidWrapper& other)
{
	A = other.A;
	B = other.B;
	C = other.C;
	D = other.D;
	return *this;
}