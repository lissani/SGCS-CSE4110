# Store Management System

편의점 관리 시스템을 위한 데이터베이스 애플리케이션입니다. MySQL 데이터베이스와 C++을 사용하여 매장 운영에 필요한 7가지 핵심 쿼리를 제공합니다.

## 기능 개요

이 시스템은 다음 7가지 주요 기능을 제공합니다:

1. **TYPE 1**: 특정 상품을 보유한 매장과 재고량 조회
2. **TYPE 2**: 각 매장별 최근 한달간 최고 판매 상품 분석
3. **TYPE 3**: 현재 분기 최고 매출 매장 조회
4. **TYPE 4**: 가장 많은 상품을 공급하는 벤더와 판매량 분석
5. **TYPE 5**: 재주문이 필요한 상품 목록 조회
6. **TYPE 6**: 충성 고객이 커피와 함께 구매하는 상위 3개 상품 분석
7. **TYPE 7**: 가맹점과 직영점의 상품 다양성 비교

## 실험 환경

- **운영체제**: Windows 11
- **컴파일러**: MinGW-w64 (g++)
- **개발환경**: Visual Studio Code
- **데이터베이스**: MySQL 8.0.42
- **MySQL Connector**: MySQL Connector C 6.1

## 시스템 요구사항

### 필수 소프트웨어
1. **MySQL Server 8.0 이상**
2. **MySQL Workbench** (데이터베이스 관리용)
3. **MinGW-w64** (g++ 컴파일러)
4. **Visual Studio Code**
5. **MySQL Connector C 6.1**

## 개발 환경 설정

### 1. MinGW-w64 설치
1. MinGW-w64 다운로드 및 설치
2. 기본 설치 경로: `C:\mingw64\`

### 2. MySQL Connector C 6.1 설치
1. MySQL 공식 사이트에서 Connector C 6.1 다운로드
2. 기본 설치 경로: `C:\Program Files\MySQL\MySQL Connector C 6.1\`

### 3. 환경 변수 설정
**시스템 속성 → 환경변수 → 시스템 변수 → PATH에 다음 항목들 추가:**
```
C:\mingw64\bin
C:\Program Files\MySQL\MySQL Connector C 6.1\bin
C:\Program Files\MySQL\MySQL Connector C 6.1\lib
C:\Program Files\MySQL\MySQL Connector C 6.1\include
```

### 4. Visual Studio Code 설정
프로젝트 루트 폴더에 `.vscode` 폴더를 생성하고 다음 파일들을 추가합니다:

**.vscode/c_cpp_properties.json:**
```json
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**",
                "C:/Program Files/MySQL/MySQL Connector C 6.1/include"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "compilerPath": "C:/mingw64/bin/g++.exe",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "windows-gcc-x64"
        }
    ],
    "version": 4
}
```

**.vscode/tasks.json:**
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "type": "cppbuild",
            "label": "C/C++: g++ MySQL 빌드",
            "command": "C:\\mingw64\\bin\\g++.exe",
            "args": [
                "-fdiagnostics-color=always",
                "-g",
                "${file}",
                "-o",
                "${fileDirname}\\${fileBasenameNoExtension}.exe"
            ],
            "options": {
                "cwd": "${fileDirname}"
            },
            "problemMatcher": ["$gcc"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "detail": "MySQL과 연결하는 g++ 컴파일러"
        }
    ]
}
```

### 5. 헤더 파일 설정
**main.cpp에서 MySQL 헤더 파일 포함:**
```cpp
#include <mysql.h>  // MySQL Connector C 헤더 파일
```

## 데이터베이스 설정

### 1. 데이터베이스 생성
```sql
CREATE DATABASE store;
USE store;
```

### 2. MySQL 인증 방식 설정 (중요!)
**MySQL Workbench에서 다음 쿼리를 실행하여 인증 오류를 방지합니다:**
```sql
ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY '1234';
FLUSH PRIVILEGES;
```

### 3. 테이블 생성 및 데이터 삽입
제공된 DDL 및 INSERT 스크립트를 순서대로 실행합니다.
- schema.sql (DDL)
- sample_data.sql (INSERT 스크립트)

### 4. 프로그램 연결 정보 설정
`main.cpp`에서 데이터베이스 연결 정보를 확인합니다:
```cpp
const char *server = "localhost";
const char *user = "root";
const char *password = "1234";  // 위에서 설정한 비밀번호
const char *database = "store";
```

### 5. SSL 연결 설정 (중요!)
MySQL 연결시 SSL 관련 오류를 방지하기 위해 다음 코드를 main.cpp의 mysql_real_connect 호출 전에 추가합니다:
```cpp
// SSL 비활성화 설정
mysql_ssl_mode sslmode = SSL_MODE_DISABLED;
if (mysql_options(conn, MYSQL_OPT_SSL_MODE, &sslmode)) {
    std::cerr << "mysql_options() failed: " << mysql_error(conn) << "\n";
    mysql_close(conn);
    return 1;
}
```

## 컴파일 및 실행

### VS Code 터미널에서 컴파일
```bash
g++ main.cpp -o main.exe -I"C:\Program Files\MySQL\MySQL Connector C 6.1\include" -L"C:\Program Files\MySQL\MySQL Connector C 6.1\lib" -lmysql
```

### 실행
```bash
.\main.exe
```

## 예시 쿼리 실행 순서

### TYPE 1 쿼리 실행 예시

**1단계: 프로그램 시작**
```bash
> .\main.exe
```

**2단계: 메뉴에서 선택**
```
---------- SELECT QUERY TYPES ----------

     1. TYPE 1
     2. TYPE 2
     3. TYPE 3
     4. TYPE 4
     5. TYPE 5
     6. TYPE 6
     7. TYPE 7
     0. QUIT

Select: 1
```

**3단계: 쿼리 유형 안내**
```
----- TYPE 1 -----
** Which stores currently carry a certain product (by UPC, name, or brand), and how much inventory do they have? **
```

**4단계: 상품 식별자 입력**
```
Enter product identifier (UPC, name, or brand): black coffee
```

**5단계: 결과 확인**
```
--- Query Result ---
store_id            store_name          upc                 product_name        stock_amount        
1                   Downtown Branch     1001234567890       black coffee        25
2                   University Store    1001234567890       black coffee        18
3                   City Center         1001234567890       black coffee        5
4                   Metro Station       1001234567890       black coffee        32
5                   Shopping Mall       1001234567890       black coffee        40
```

**6단계: 계속 사용 또는 종료**
메뉴가 다시 표시되어 다른 쿼리를 실행하거나 `0`을 입력하여 종료할 수 있습니다.