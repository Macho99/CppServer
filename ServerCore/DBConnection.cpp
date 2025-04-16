#include "pch.h"
#include "DBConnection.h"

/*--------------------
	DBConnection
---------------------*/

bool DBConnection::Connect(SQLHENV henv, const WCHAR* connectionString)
{
	if (::SQLAllocHandle(SQL_HANDLE_DBC, henv, &_connection) != SQL_SUCCESS)
		return false;

	WCHAR stringBuffer[MAX_PATH] = { 0 };
	::wcscpy_s(stringBuffer, connectionString);

	WCHAR resultString[MAX_PATH] = { 0 };
	SQLSMALLINT resultStringLength = 0;

	SQLRETURN ret = ::SQLDriverConnectW(
		_connection,
		NULL,
		reinterpret_cast<SQLWCHAR*>(stringBuffer),
		_countof(stringBuffer),
		OUT reinterpret_cast<SQLWCHAR*>(resultString),
		_countof(resultString),
		OUT & resultStringLength,
		SQL_DRIVER_NOPROMPT
	);
	
	if (::SQLAllocHandle(SQL_HANDLE_STMT, _connection, &_statement) != SQL_SUCCESS)
		return false;

	return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

void DBConnection::Clear()
{
	if (_connection != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_DBC, _connection);
		_connection = SQL_NULL_HANDLE;
	}

	if (_statement != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_STMT, _statement);
		_statement = SQL_NULL_HANDLE;
	}
}

bool DBConnection::Execute(const WCHAR* query)
{
	SQLRETURN ret = ::SQLExecDirectW(_statement, (SQLWCHAR*)query, SQL_NTS);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return true;

	HandleError(ret);
	return false;
}

bool DBConnection::Fetch()
{
	SQLRETURN ret = ::SQLFetch(_statement);

	switch (ret)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
		return true;
	case SQL_NO_DATA:
		return false;
	case SQL_ERROR:
		HandleError(ret);
		return false;
	default:
		return true;
	}
}

int32 DBConnection::GetRowCount()
{
	SQLLEN count = 0;
	SQLRETURN ret = ::SQLRowCount(_statement, OUT & count);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return static_cast<int32>(count);

	return -1;
}

void DBConnection::Unbind()
{
	::SQLFreeStmt(_statement, SQL_UNBIND);
	::SQLFreeStmt(_statement, SQL_RESET_PARAMS);
	::SQLFreeStmt(_statement, SQL_CLOSE);
}

bool DBConnection::BindParam(int32 paramIndex, bool* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, size32(bool), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, float* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_FLOAT, SQL_REAL, 0, value, index);
}

bool DBConnection::BindParam(int32 paramIndex, double* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_DOUBLE, SQL_DOUBLE, 0, value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int8* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, size32(int8), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int16* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_SHORT, SQL_SMALLINT, size32(int16), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int32* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_LONG, SQL_INTEGER, size32(int32), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int64* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_SBIGINT, SQL_BIGINT, size32(int64), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, size32(TIMESTAMP_STRUCT), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, const WCHAR* str, SQLLEN* index)
{
	SQLULEN size = static_cast<SQLULEN>((::wcslen(str) + 1) * sizeof(WCHAR));
	*index = SQL_NTS;

	if(size > WVARCHAR_MAX)
		return BindParam(paramIndex, SQL_C_WCHAR, SQL_WLONGVARCHAR, size, (SQLPOINTER)str, index);
	else
		return BindParam(paramIndex, SQL_C_WCHAR, SQL_WVARCHAR, size, (SQLPOINTER)str, index);
}

bool DBConnection::BindParam(int32 paramIndex, const BYTE* bin, int32 size, SQLLEN* index)
{
	if (bin == nullptr)
	{
		*index = SQL_NULL_DATA;
		size = 1;
	}
	else
	{
		*index = size;
	}

	if (size > BINARY_MAX)
		return BindParam(paramIndex, SQL_C_BINARY, SQL_LONGVARBINARY, size, (BYTE*)bin, index);
	else
		return BindParam(paramIndex, SQL_C_BINARY, SQL_VARBINARY, size, (BYTE*)bin, index);
}

bool DBConnection::BindCol(int32 colIndex, bool* value, SQLLEN* index)
{
	return BindCol(colIndex, SQL_C_TINYINT, size32(bool), value, index);
}

bool DBConnection::BindCol(int32 colIndex, float* value, SQLLEN* index)
{
	return BindCol(colIndex, SQL_C_FLOAT, size32(float), value, index);
}

bool DBConnection::BindCol(int32 colIndex, double* value, SQLLEN* index)
{
	return BindCol(colIndex, SQL_C_DOUBLE, size32(double), value, index);
}

bool DBConnection::BindCol(int32 colIndex, int8* value, SQLLEN* index)
{
	return BindCol(colIndex, SQL_C_TINYINT, size32(int8), value, index);
}

bool DBConnection::BindCol(int32 colIndex, int16* value, SQLLEN* index)
{
	return BindCol(colIndex, SQL_C_SHORT, size32(int16), value, index);
}

bool DBConnection::BindCol(int32 colIndex, int32* value, SQLLEN* index)
{
	return BindCol(colIndex, SQL_C_LONG, size32(int32), value, index);
}

bool DBConnection::BindCol(int32 colIndex, int64* value, SQLLEN* index)
{
	return BindCol(colIndex, SQL_C_SBIGINT, size32(int64), value, index);
}

bool DBConnection::BindCol(int32 colIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
	return BindCol(colIndex, SQL_C_TYPE_TIMESTAMP, size32(TIMESTAMP_STRUCT), value, index);
}

bool DBConnection::BindCol(int32 colIndex, WCHAR* str, int32 size, SQLLEN* index)
{
	return BindCol(colIndex, SQL_C_WCHAR, size, str, index);
}

bool DBConnection::BindCol(int32 colIndex, BYTE* bin, int32 size, SQLLEN* index)
{
	return BindCol(colIndex, SQL_C_BINARY, size, bin, index);
}

bool DBConnection::BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index)
{
	SQLRETURN ret = ::SQLBindParameter(
		_statement,
		paramIndex,
		SQL_PARAM_INPUT,
		cType,
		sqlType,
		len,
		0,
		ptr,
		0,
		index
	);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return true;

	HandleError(ret);
	return false;
}

bool DBConnection::BindCol(SQLUSMALLINT colIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index)
{
	SQLRETURN ret = ::SQLBindCol(
		_statement,
		colIndex,
		cType,
		value,
		len,
		index
	);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return true;

	HandleError(ret);
	return false;
}

void DBConnection::HandleError(SQLRETURN ret)
{
	if (ret == SQL_SUCCESS)
		return;

	SQLSMALLINT index = 1;
	SQLWCHAR sqlState[MAX_PATH] = { 0 };
	SQLINTEGER nativeError = 0;
	SQLWCHAR errMsg[MAX_PATH] = { 0 };
	SQLSMALLINT errMsgLength = 0;
	SQLRETURN errorRet = 0;

	while (true)
	{
		errorRet = ::SQLGetDiagRecW(
			SQL_HANDLE_STMT,
			_statement,
			index,
			sqlState,
			OUT &nativeError,
			errMsg,
			_countof(errMsg),
			OUT &errMsgLength
		);

		if (errorRet == SQL_NO_DATA)
			break;

		if (errorRet != SQL_SUCCESS && errorRet != SQL_SUCCESS_WITH_INFO)
			break;

		wcout.imbue(locale("kor"));
		wcout << errMsg << endl;

		index++;
	}
}
