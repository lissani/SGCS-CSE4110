-- drop table PurchaseDetail;
-- drop table Inventory;
-- drop table Store;
-- drop table Customer;
-- drop table Product;
-- drop table Vendor;
-- drop table SalesTransaction;
-- drop table VendorSupply;
-- drop table ProductPrice;

CREATE TABLE Store (
    store_id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    address VARCHAR(255) NOT NULL,
    open_time TIME NOT NULL,
    close_time TIME NOT NULL,
    ownership_type ENUM('corporate', 'franchise') NOT NULL
);

CREATE TABLE Customer (
    customer_id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    phone VARCHAR(20) NOT NULL UNIQUE,
    email VARCHAR(150) UNIQUE,
    loyalty_level ENUM('bronze', 'silver', 'gold', 'platinum', 'vip') DEFAULT 'bronze',
    
    -- 제약조건
    CONSTRAINT chk_phone_format CHECK (phone REGEXP '^[0-9-+()\\s]+$'),
    CONSTRAINT chk_email_format CHECK (email REGEXP '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$')
);

CREATE TABLE Product (
    upc VARCHAR(20) PRIMARY KEY,
    name VARCHAR(150) NOT NULL,
    brand VARCHAR(100),
    price DECIMAL(8,2) NOT NULL CHECK (price > 0),
    size VARCHAR(50),
    package_type VARCHAR(50)
);

CREATE TABLE Vendor (
    vendor_id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(150) NOT NULL,
    contact_info JSON
);

CREATE TABLE Inventory (
    store_id INT,
    upc VARCHAR(20),
    stock_amount INT NOT NULL DEFAULT 0 CHECK (stock_amount >= 0),
    reorder_threshold INT NOT NULL DEFAULT 10 CHECK (reorder_threshold >= 0),
    reorder_quantity INT NOT NULL DEFAULT 50 CHECK (reorder_quantity > 0),
    
    -- 복합 기본키
    PRIMARY KEY (store_id, upc),
    
    -- 외래키 제약조건
    FOREIGN KEY (store_id) REFERENCES Store(store_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (upc) REFERENCES Product(upc) 
        ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE TABLE SalesTransaction (
    transaction_id INT AUTO_INCREMENT PRIMARY KEY,
    payment_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    payment_method ENUM('cash', 'card', 'mobile', 'points') NOT NULL,
    customer_id INT,
    store_id INT NOT NULL,

    -- 외래키 제약조건
    FOREIGN KEY (customer_id) REFERENCES Customer(customer_id) 
        ON DELETE SET NULL ON UPDATE CASCADE,
    FOREIGN KEY (store_id) REFERENCES Store(store_id) 
        ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE TABLE PurchaseDetail (
    transaction_id INT,
    upc VARCHAR(20),
    quantity INT NOT NULL CHECK (quantity > 0),

    -- 복합 기본키
    PRIMARY KEY (transaction_id, upc),
    
    -- 외래키 제약조건
    FOREIGN KEY (transaction_id) REFERENCES SalesTransaction(transaction_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (upc) REFERENCES Product(upc) 
        ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE TABLE VendorSupply (
    vendor_id INT,
    upc VARCHAR(20),
    supply_quantity INT NOT NULL CHECK (supply_quantity > 0),
    
    -- 복합 기본키
    PRIMARY KEY (vendor_id, upc),
    
    -- 외래키 제약조건
    FOREIGN KEY (vendor_id) REFERENCES Vendor(vendor_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (upc) REFERENCES Product(upc) 
        ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE TABLE ProductPrice (
    upc VARCHAR(20) PRIMARY KEY,
    supply_price DECIMAL(8,2) NOT NULL CHECK (supply_price > 0),

    -- 외래키 제약조건
    FOREIGN KEY (upc) REFERENCES Product(upc) 
        ON DELETE CASCADE ON UPDATE CASCADE
);

