# 🚀 学生成绩管理系统开发日志：第一阶段（终极版）

**日期：** 2026-03-09 **环境：** Arch Linux / Docker / openGauss 5.0.0 **目标：** 完成三表建模，掌握 SQL 别名、多表联查及数据维护。

------

## 1. 数据库模型：三表架构

为了实现“多对多”关系，我们不把成绩直接写在学生表里，而是通过一张中间表来连接。

- **`student_info` (别名 `s`)**: 存放学生档案。
- **`course_info` (别名 `c`)**: 存放课程清单。
- **`student_grades` (别名 `g`)**: 核心纽带，存放“谁选了哪课，考了几分”。

------

## 2. DDL：构建数据库地基

在 openGauss 中执行以下代码，注意其中的约束条件：

```sql
-- 学生表
CREATE TABLE student_info (
    stu_id      VARCHAR(20) PRIMARY KEY,
    name        VARCHAR(50) NOT NULL,
    age         INT CHECK (age >= 10 AND age <= 100)
);

-- 课程表
CREATE TABLE course_info (
    course_id   INT PRIMARY KEY,
    course_name VARCHAR(100) UNIQUE NOT NULL,
    credits     INT DEFAULT 3
);

-- 成绩表（关联表）
CREATE TABLE student_grades (
    stu_id      VARCHAR(20) REFERENCES student_info(stu_id),
    course_id   INT REFERENCES course_info(course_id),
    score       DECIMAL(5,2) CHECK (score >= 0.00 AND score <= 100.00),
    PRIMARY KEY (stu_id, course_id) -- 复合主键：防止同一人重复选同一门课
);
```

------

## 3. DML：数据的增、删、改、查

### 3.1 增 (Insert) —— 注意顺序

**原则：** 必须先有学生和课程，才能录入成绩。

```sql
-- 录入基础数据
INSERT INTO student_info (stu_id, name, age) VALUES ('S2026', 'Meowzart', 20);
INSERT INTO course_info (course_id, course_name) VALUES (101, 'Linux C Programming');

-- 录入关联数据
INSERT INTO student_grades (stu_id, course_id, score) VALUES ('S2026', 101, 98.5);
```

### 3.2 查 (Select) —— 别名与 JOIN 的艺术

这里使用了别名（Alias）：`s` 代表学生，`c` 代表课程，`g` 代表成绩。

```sql
SELECT 
    s.stu_id, 
    s.name, 
    c.course_name, 
    g.score
FROM student_info s
JOIN student_grades g ON s.stu_id = g.stu_id
```

### 3.3 改 (Update) —— 精准定位

修改特定学生、特定课程的成绩。

```sql
UPDATE student_grades 
SET score = 100 
WHERE stu_id = 'S2026' AND course_id = 101;
```

### 3.4 删 (Delete) —— 级联意识

**坑：** 不能直接删学生！因为成绩表里还引用着他的学号。

```sql
-- 1. 先删掉该学生的成绩记录（清理外键引用）
DELETE FROM student_grades WHERE stu_id = 'S2026';

-- 2. 再删掉该学生的基本信息
DELETE FROM student_info WHERE stu_id = 'S2026';
```

------

## 4. 核心心得（必看）

1. **关于 `s, c, g`**：它们就像代码里的变量名。通过 `s.name`，SQL 引擎能瞬间明白：“去学生表找名字，别去课程表找”。这在处理多表同名字段（比如两个表都有 `id`）时是救命的。
2. **数据安全**：在执行 `UPDATE` 和 `DELETE` 时，**永远先看一眼 `WHERE` 子句**。没有 `WHERE` 的删除会导致全库清空。
3. **Arch 环境**：由于是 Docker 部署，记得定期用 `docker cp` 备份你的 SQL 脚本到宿主机。