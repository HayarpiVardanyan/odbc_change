#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <iomanip> 

void printErrorMessage(SQLHANDLE handle, SQLSMALLINT handleType) {
    SQLCHAR sqlstate[6];
    SQLCHAR errmsg[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER nativeerr;
    SQLSMALLINT length;
    while (SQLGetDiagRec(handleType, handle, 1, sqlstate, &nativeerr, errmsg, SQL_MAX_MESSAGE_LENGTH, &length) != SQL_NO_DATA) {
        std::cerr << "SQLSTATE: " << sqlstate << " Native Error: " << nativeerr << " Message: " << errmsg << std::endl;
    }
}

void executeQuery(SQLHDBC hdbc, const std::string& query) {
    SQLHSTMT hstmt;
    SQLRETURN ret;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    ret = SQLExecDirect(hstmt, (SQLCHAR *)query.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error executing query: " << query << std::endl;
        printErrorMessage(hstmt, SQL_HANDLE_STMT);
    } else {
        SQLSMALLINT numCols;
        ret = SQLNumResultCols(hstmt, &numCols);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            std::cerr << "Error getting number of columns for query: " << query << std::endl;
            printErrorMessage(hstmt, SQL_HANDLE_STMT);
        } else {
            std::cout << "Query executed successfully: " << query << std::endl;
            if (numCols > 0) {
                std::cout << std::left << std::setw(12) << "hospital_id" << " | ";
                std::cout << std::left << std::setw(30) << "h_name" << " | ";
                std::cout << std::left << std::setw(30) << "address" << " | ";
                std::cout << std::left << std::setw(13) << "phone_number" << std::endl;
                std::cout << std::string(90, '-') << std::endl;
                SQLCHAR colData[256];
                SQLLEN indicator;
                int numRowsAffected = 0;
                while (SQLFetch(hstmt) == SQL_SUCCESS) {
                    numRowsAffected++;
                    for (int i = 1; i <= numCols; ++i) {
                        ret = SQLGetData(hstmt, i, SQL_CHAR, colData, sizeof(colData), &indicator);
                        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                            if (indicator != SQL_NULL_DATA) {
                                std::cout << std::left << std::setw(12) << colData << " | ";
                            } else {
                                std::cout << std::left << std::setw(12) << "NULL" << " | ";
                            }
                        } else {
                            std::cerr << "Error fetching column data for query: " << query << std::endl;
                            printErrorMessage(hstmt, SQL_HANDLE_STMT);
                        }
                    }
                    std::cout << std::endl;
                }
                std::cout << "Number of rows affected: " << numRowsAffected << std::endl;
            }
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}

int main() {

    SQLHENV henv; 
    SQLHDBC hdbc; 
    SQLRETURN ret;

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

    ret = SQLDriverConnect(hdbc, NULL, (SQLCHAR *)"DRIVER={PostgreSQL ANSI};Servername=localhost;Port=5432;Database=hospital_network;Username=hospital;Password=123;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printErrorMessage(hdbc, SQL_HANDLE_DBC);
        return 1;
    }

    executeQuery(hdbc, "SELECT * FROM Doctor");

    executeQuery(hdbc, "SELECT * FROM Hospital WHERE address LIKE '%Texas%' OR address LIKE '%New York%'");

    executeQuery(hdbc, "INSERT INTO Doctor (doctor_id, department_id, doctor_name, phone_number, start_working, end_working) VALUES (210, 102, 'Dr. Parker', '5553333', '08:00:00', '16:00:00')");

    executeQuery(hdbc, "UPDATE Doctor SET doctor_name = 'Dr. Parker, MD' WHERE doctor_id = 209");

    executeQuery(hdbc, "DELETE FROM Doctor WHERE doctor_id = 209");

    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return 0;
}