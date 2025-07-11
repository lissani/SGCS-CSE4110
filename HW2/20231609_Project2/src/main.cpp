#include <iostream>
#include <mysql.h> // 헤더 파일 폴더 구조로 인해 경로 수정
#include <iomanip> // 깔끔한 출력을 위해 추가한 표준 라이브러리 헤더

void handleType1Q(MYSQL* conn){
    // 사용자 입력에 따라 점포 + 재고 출력하는 쿼리
    std::cout << "----- TYPE 1 -----\n";
    std::cout << "** Which stores currently carry a certain product (by UPC, name, or brand), and how much inventory do they have? **\n";

    std::string input;
    std::cout << "Enter product identifier (UPC, name, or brand): ";
    std::cin.ignore();
    std::getline(std::cin, input); // 띄어쓰기 포함된 입력처리

    // 1. Statement 초기화
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    std::string query =
        "SELECT s.store_id, s.name AS store_name, p.upc, p.name AS product_name, i.stock_amount "
        "FROM Inventory i "
        "JOIN Store s ON i.store_id = s.store_id "
        "JOIN Product p ON i.upc = p.upc "
        "WHERE p.upc = ? OR p.name = ? OR p.brand = ?;";

    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        std::cerr << "Prepare failed: " << mysql_stmt_error(stmt) << "\n";
        mysql_stmt_close(stmt);
        return;
    }

    // 2. 파라미터 바인딩
    MYSQL_BIND bind[3]{};
    unsigned long str_len = input.length();

    for (int i = 0; i < 3; ++i) {
        bind[i].buffer_type = MYSQL_TYPE_STRING;
        bind[i].buffer = (void*)input.c_str();
        bind[i].buffer_length = str_len;
        bind[i].length = &str_len;
    }

    if (mysql_stmt_bind_param(stmt, bind)) {
        std::cerr << "Bind failed: " << mysql_stmt_error(stmt) << "\n";
        mysql_stmt_close(stmt);
        return;
    }

    // 3. 실행
    if (mysql_stmt_execute(stmt)) {
        std::cerr << "Execute failed: " << mysql_stmt_error(stmt) << "\n";
        mysql_stmt_close(stmt);
        return;
    }

    // 4. 결과 바인딩
    MYSQL_BIND result[5]{};
    int store_id;
    char store_name[100], upc[30], product_name[100];
    int stock_amount;
    unsigned long name_len[5];

    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &store_id;

    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = store_name;
    result[1].buffer_length = sizeof(store_name);
    result[1].length = &name_len[1];

    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = upc;
    result[2].buffer_length = sizeof(upc);
    result[2].length = &name_len[2];

    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = product_name;
    result[3].buffer_length = sizeof(product_name);
    result[3].length = &name_len[3];

    result[4].buffer_type = MYSQL_TYPE_LONG;
    result[4].buffer = &stock_amount;

    if (mysql_stmt_bind_result(stmt, result)) {
        std::cerr << "Bind result failed: " << mysql_stmt_error(stmt) << "\n";
        mysql_stmt_close(stmt);
        return;
    }

    // 5. 결과 출력
    std::cout << "\n--- Query Result ---\n";
    std::cout << std::left << std::setw(12) << "store_id"
              << std::setw(20) << "store_name"
              << std::setw(20) << "upc"
              << std::setw(25) << "product_name"
              << std::setw(10) << "stock" << "\n";

    while (mysql_stmt_fetch(stmt) == 0) {
        store_name[name_len[1]] = '\0';
        upc[name_len[2]] = '\0';
        product_name[name_len[3]] = '\0';

        std::cout << std::left << std::setw(12) << store_id
                  << std::setw(20) << store_name
                  << std::setw(20) << upc
                  << std::setw(25) << product_name
                  << std::setw(10) << stock_amount << "\n";
    }

    mysql_stmt_close(stmt);
}

void handleType2Q(MYSQL* conn){
    // 각 store에서 지난 한 달 동안 판매량이 가장 많은 상품 출력
    std::cout << "----- TYPE 2 -----\n";
    std::cout << "** Which products have the highest sales volume in each store over the past month? **\n";

    std::string query =
        "SELECT t.store_id, s.name AS store_name, p.upc, p.name AS product_name, SUM(d.quantity) AS total_quantity "
        "FROM SalesTransaction t "
        "JOIN PurchaseDetail d ON t.transaction_id = d.transaction_id "
        "JOIN Product p ON d.upc = p.upc "
        "JOIN Store s ON t.store_id = s.store_id "
        "WHERE t.payment_time >= DATE_SUB(CURDATE(), INTERVAL 1 MONTH) "
        "GROUP BY t.store_id, p.upc "
        "HAVING total_quantity = ( "
        "    SELECT MAX(sub.total_quantity) "
        "    FROM ( "
        "        SELECT SUM(d2.quantity) AS total_quantity "
        "        FROM SalesTransaction t2 "
        "        JOIN PurchaseDetail d2 ON t2.transaction_id = d2.transaction_id "
        "        WHERE t2.store_id = t.store_id "
        "          AND t2.payment_time >= DATE_SUB(CURDATE(), INTERVAL 1 MONTH) "
        "        GROUP BY d2.upc "
        "    ) AS sub "
        ") "
        "ORDER BY t.store_id;";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn) << "\n";
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Result retrieval failed: " << mysql_error(conn) << "\n";
        return;
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_ROW row;
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    std::cout << "\n--- Query Result ---\n";
    for (int i = 0; i < num_fields; ++i) {
        std::cout << std::left << std::setw(20) << fields[i].name;  // 열 너비 고정
    }
    std::cout << "\n";

    while ((row = mysql_fetch_row(res))) {
        for (int i = 0; i < num_fields; ++i) {
            std::cout << std::left << std::setw(20) << (row[i] ? row[i] : "NULL");
        }
        std::cout << "\n";
    }

    mysql_free_result(res);
}

void handleType3Q(MYSQL* conn){
    // revenue = sum(quantity * price)
    std::cout << "----- TYPE 3 -----\n";
    std::cout << "** Which store has generated the highest overall revenue this quarter? **\n";

    std::string query =
        "SELECT t.store_id, s.name AS store_name, SUM(d.quantity * p.price) AS revenue "
        "FROM SalesTransaction t "
        "JOIN PurchaseDetail d ON t.transaction_id = d.transaction_id "
        "JOIN Product p ON d.upc = p.upc "
        "JOIN Store s ON t.store_id = s.store_id "
        "WHERE QUARTER(t.payment_time) = QUARTER(CURDATE()) AND YEAR(t.payment_time) = YEAR(CURDATE()) "
        "GROUP BY t.store_id "
        "ORDER BY revenue DESC "
        "LIMIT 1;";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn) << "\n";
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Result retrieval failed: " << mysql_error(conn) << "\n";
        return;
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_ROW row;
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    std::cout << "\n--- Query Result ---\n";
    for (int i = 0; i < num_fields; ++i) {
        std::cout << std::left << std::setw(20) << fields[i].name;  // 열 너비 고정
    }
    std::cout << "\n";

    while ((row = mysql_fetch_row(res))) {
        for (int i = 0; i < num_fields; ++i) {
            std::cout << std::left << std::setw(20) << (row[i] ? row[i] : "NULL");
        }
        std::cout << "\n";
    }

    mysql_free_result(res);
}

void handleType4Q(MYSQL* conn){
    // 가장 많은 상품을 공급한 vendor
    // 그 vendor가 공급한 상품들이 얼마나 판매되었는지
    std::cout << "----- TYPE 4 -----\n";
    std::cout << "** Which vendor supplies the most products across the chain, and how many total units have been sold? **\n";

    std::string query =
        "SELECT v.vendor_id, v.name, COUNT(DISTINCT vs.upc) AS num_products, "
        "SUM(pd.quantity) AS total_units "
        "FROM Vendor v "
        "JOIN VendorSupply vs ON v.vendor_id = vs.vendor_id "
        "LEFT JOIN PurchaseDetail pd ON vs.upc = pd.upc "
        "GROUP BY v.vendor_id "
        "ORDER BY num_products DESC "
        "LIMIT 1; " ;

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn) << "\n";
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Result retrieval failed: " << mysql_error(conn) << "\n";
        return;
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_ROW row;
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    std::cout << "\n--- Query Result ---\n";
    for (int i = 0; i < num_fields; ++i) {
        std::cout << std::left << std::setw(20) << fields[i].name;  // 열 너비 고정
    }
    std::cout << "\n";

    while ((row = mysql_fetch_row(res))) {
        for (int i = 0; i < num_fields; ++i) {
            std::cout << std::left << std::setw(20) << (row[i] ? row[i] : "NULL");
        }
        std::cout << "\n";
    }

    mysql_free_result(res);
}

void handleType5Q(MYSQL* conn){
    // 각 store 별로 재고가 재주문 임계값보다 낮은 상품을 찾기
    std::cout << "----- TYPE 5 -----\n";
    std::cout << "** Which products in each store are below the reorder threshold and need restocking? **\n";

    std::string query =
        "SELECT s.store_id, s.name AS store_name, "
        "p.upc, p.name AS product_name, "
        "i.stock_amount, i.reorder_threshold "
        "FROM Inventory i "
        "JOIN Store s ON i.store_id = s.store_id "
        "JOIN Product p ON i.upc = p.upc "
        "WHERE i.stock_amount < i.reorder_threshold "
        "ORDER BY s.store_id;";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn) << "\n";
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Result retrieval failed: " << mysql_error(conn) << "\n";
        return;
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_ROW row;
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    std::cout << "\n--- Query Result ---\n";
    for (int i = 0; i < num_fields; ++i) {
        std::cout << std::left << std::setw(20) << fields[i].name;  // 열 너비 고정
    }
    std::cout << "\n";

    while ((row = mysql_fetch_row(res))) {
        for (int i = 0; i < num_fields; ++i) {
            std::cout << std::left << std::setw(20) << (row[i] ? row[i] : "NULL");
        }
        std::cout << "\n";
    }

    mysql_free_result(res);
}

void handleType6Q(MYSQL* conn){
    std::cout << "----- TYPE 6 -----\n";
    std::cout << "** List the top 3 items that loyalty program customers typically purchase with coffee. **\n";

    std::string input;
    // db에 존재하는 coffee product는 black coffee와 milk coffee만 존재한다고 가정
    // loyalty program customers는 loyalty_level != 'bronze'라고 가정
    std::cout << "Enter a product name (black coffee or milk coffee): ";
    std::cin.ignore();
    std::getline(std::cin, input); // 띄어쓰기 포함된 입력처리

    // SQL injection 방지
    if (input != "black coffee" && input != "milk coffee") {
        std::cout << "You can only use 'black coffee' or 'milk coffee' as a coffee product." << "\n";
        return;
    }

    std::string query =
        "SELECT p2.name AS product_name, COUNT(*) AS co_purchase_count "
        "FROM PurchaseDetail pd1 "
        "JOIN SalesTransaction t ON pd1.transaction_id = t.transaction_id "
        "JOIN Customer c ON t.customer_id = c.customer_id "
        "JOIN Product p1 ON pd1.upc = p1.upc "
        "JOIN PurchaseDetail pd2 ON pd1.transaction_id = pd2.transaction_id "
        "JOIN Product p2 ON pd2.upc = p2.upc "
        "WHERE p1.name = '" + input + "' "
        "  AND p2.name != '" + input + "' "
        "  AND c.loyalty_level != 'bronze' "
        "GROUP BY p2.name "
        "ORDER BY co_purchase_count DESC "
        "LIMIT 3;";
        
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn) << "\n";
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Result retrieval failed: " << mysql_error(conn) << "\n";
        return;
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_ROW row;
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    std::cout << "\n--- Query Result ---\n";
    for (int i = 0; i < num_fields; ++i) {
        std::cout << std::left << std::setw(20) << fields[i].name;  // 열 너비 고정
    }
    std::cout << "\n";

    while ((row = mysql_fetch_row(res))) {
        for (int i = 0; i < num_fields; ++i) {
            std::cout << std::left << std::setw(20) << (row[i] ? row[i] : "NULL");
        }
        std::cout << "\n";
    }

    mysql_free_result(res);
}

void handleType7Q(MYSQL* conn){
    // franchise store 중 가장 많은 종류 보유한 점포 찾기
    // 이후 corporate store 정체의 평균 상품 종류 수와 비교
    std::cout << "----- TYPE 7 -----\n";
    std::cout << "** Among franchise-owned stores, which one offers the widest variety of products, and how does that compare to corporate-owned stores? **\n";

    std::string query =
        "SELECT "
        "    fs.store_id AS f_store_id, "
        "    fs.name AS f_store_name, "
        "    fs_variety.product_count AS f_product_variety, "
        "    cs.store_id AS c_store_id, "
        "    cs.name AS c_store_name, "
        "    cs_variety.product_count AS c_product_variety "
        "FROM ( "
        "    SELECT i.store_id, COUNT(DISTINCT i.upc) AS product_count "
        "    FROM Inventory i "
        "    JOIN Store s ON i.store_id = s.store_id "
        "    WHERE s.ownership_type = 'franchise' "
        "    GROUP BY i.store_id "
        "    ORDER BY product_count DESC "
        "    LIMIT 1 "
        ") AS fs_variety "
        "JOIN Store fs ON fs.store_id = fs_variety.store_id "
        "JOIN ( "
        "    SELECT i.store_id, COUNT(DISTINCT i.upc) AS product_count "
        "    FROM Inventory i "
        "    JOIN Store s ON i.store_id = s.store_id "
        "    WHERE s.ownership_type = 'corporate' "
        "    GROUP BY i.store_id "
        "    ORDER BY product_count DESC "
        "    LIMIT 1 "
        ") AS cs_variety "
        "JOIN Store cs ON cs.store_id = cs_variety.store_id;";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn) << "\n";
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Result retrieval failed: " << mysql_error(conn) << "\n";
        return;
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_ROW row;
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    std::cout << "\n--- Query Result ---\n";
    for (int i = 0; i < num_fields; ++i) {
        std::cout << std::left << std::setw(20) << fields[i].name;  // 열 너비 고정
    }
    std::cout << "\n";

    while ((row = mysql_fetch_row(res))) {
        for (int i = 0; i < num_fields; ++i) {
            std::cout << std::left << std::setw(20) << (row[i] ? row[i] : "NULL");
        }
        std::cout << "\n";
    }

    mysql_free_result(res);
}

void QueryChoice(MYSQL* conn){
    int choice;
    while(true){
        std::cout << "---------- SELECT QUERY TYPES ----------\n\n";
        std::cout << "     1. TYPE 1\n";
        std::cout << "     2. TYPE 2\n";
        std::cout << "     3. TYPE 3\n";
        std::cout << "     4. TYPE 4\n";
        std::cout << "     5. TYPE 5\n";
        std::cout << "     6. TYPE 6\n";
        std::cout << "     7. TYPE 7\n";
        std::cout << "     0. QUIT\n\n";

        std::cout << "Select: ";
        std::cin >> choice;

        switch (choice){
        case 1:
            handleType1Q(conn);
            std::cout << "\n";
            break;
        case 2:
            handleType2Q(conn);
            std::cout << "\n";
            break;
        case 3:
            handleType3Q(conn);
            std::cout << "\n";
            break;
        case 4:
            handleType4Q(conn);
            std::cout << "\n";
            break;
        case 5:
            handleType5Q(conn);
            std::cout << "\n";
            break;
        case 6:
            handleType6Q(conn);
            std::cout << "\n";
            break;
        case 7:
            handleType7Q(conn);
            std::cout << "\n";
            break;
        case 0:
            // 프로그램 종료
            std::cout << "Program terminated.";
            return;
        default:
            std::cout << "Invalid choice.";
        }
    }
}

int main() {
    MYSQL *conn;
    //MYSQL_RES *res;
    //MYSQL_ROW row;

    const char *server = "localhost";
    const char *user = "root";
    const char *password = "1234"; // 여기에 비밀번호 입력 1234로 변경?
    const char *database = "store"; // 여기에 데이터베이스 이름 입력

    // MySQL 초기화
    conn = mysql_init(nullptr);
    if (conn == nullptr) {
        std::cerr << "mysql_init() failed\n";
        return 1;
    }

    // SSL 비활성화 설정 (mysql_real_connect 호출 전에 추가)
    mysql_ssl_mode sslmode = SSL_MODE_DISABLED;
    if (mysql_options(conn, MYSQL_OPT_SSL_MODE, &sslmode)) {
        std::cerr << "mysql_options() failed: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return 1;
    }
    
    // MySQL 서버 연결
    if (mysql_real_connect(conn, server, user, password, database, 0, nullptr, 0) == nullptr) {
        std::cerr << "mysql_real_connect() failed\n";
        mysql_close(conn);
        return 1;
    }

    QueryChoice(conn);

    // 리소스 해제
    mysql_close(conn);

    return 0;
}
