//
// Created by chen on 2022/7/30.
//

#include "UserDao.h"

UserDao::UserDao() {
	mysql = nullptr;
}

GoBang::Player UserDao::queryOneUser(connection_pool *pool, const std::string& name, const std::string& passwd, bool &success) {
	connectionRAII conn(&mysql, pool);
	char sqlstr[1024];
	memset(sqlstr, 0, sizeof(sqlstr));
	sprintf(sqlstr, "select uid,name,points from user where name = '%s' and passwd = '%s'", name.c_str(), passwd.c_str());
	if(mysql_query(mysql, sqlstr)){
		//出错
		std::cout << "SELECT error:" <<  mysql_error(mysql) << '\n';
		success = false;
		return GoBang::Player();
	}
	//从表中检索完整的结果集
	MYSQL_RES *result = mysql_store_result(mysql);

	GoBang::Player p;
	int flag = 0;
	//从结果集中获取下一行，将对应的用户名和密码，存入map中
	while (MYSQL_ROW row = mysql_fetch_row(result))
	{
		flag = 1;
		p.set_uid(row[0]);
		p.set_name(row[1]);
		p.set_points(atoi(row[2]));
	}
	mysql_free_result(result);
	if(flag == 1) success = true;
	return p;
}

std::vector<GoBang::Player> UserDao::querySomeUser(connection_pool *pool, int count, int start) {
	std::vector<GoBang::Player> res;
	connectionRAII conn(&mysql, pool);
	char sqlstr[1024];
	memset(sqlstr, 0, sizeof(sqlstr));
	sprintf(sqlstr, "select uid,name,points from user where points < %d order by points desc limit %d", start, count);
	if(mysql_query(mysql, sqlstr)){
		std::cout << "SELECT error:" <<  mysql_error(mysql) << '\n';
		return res;
	}
	//从表中检索完整的结果集
	MYSQL_RES *result = mysql_store_result(mysql);


	//从结果集中获取下一行，将对应的用户名和密码，存入map中
	while (MYSQL_ROW row = mysql_fetch_row(result))
	{
		GoBang::Player p;
		p.set_uid(row[0]);
		p.set_name(row[1]);
		p.set_points(atoi(row[2]));
		res.emplace_back(p);
	}
	mysql_free_result(result);
	return res;
}

int UserDao::insertUser(connection_pool *pool, const std::string& name, const std::string& passwd) {
	connectionRAII conn(&mysql, pool);
	char sqlstr[1024];
	memset(sqlstr, 0, sizeof(sqlstr));
	//查询有没有相同用户名的
	sprintf(sqlstr, "select uid from user where name = '%s'",name.c_str());
	if(mysql_query(mysql, sqlstr)){
		std::cout << "SELECT error:" <<  mysql_error(mysql) << '\n';
		return 0;
	}

	//从表中检索完整的结果集
	MYSQL_RES *result = mysql_store_result(mysql);

	int flag = 0;
	//从结果集中获取下一行
	while (MYSQL_ROW row = mysql_fetch_row(result))
	{
		flag = 1;
		break;
	}
	mysql_free_result(result);
	if(flag == 1) return -1;


	memset(sqlstr, 0, sizeof(sqlstr));
	sprintf(sqlstr, "insert into user(name,passwd) values('%s','%s')",name.c_str(), passwd.c_str());
	if(mysql_query(mysql, sqlstr)){
		std::cout << "SELECT error:" <<  mysql_error(mysql) << '\n';
		return 0;
	}
	return 1;
}

bool UserDao::updateUser(connection_pool *pool, const std::string& uid, int points) {
	connectionRAII conn(&mysql, pool);
	char sqlstr[1024];
	memset(sqlstr, 0, sizeof(sqlstr));
	sprintf(sqlstr, "update user set points = %d where uid = '%s'", points, uid.c_str());
	if(mysql_query(mysql, sqlstr)){
		std::cout << "SELECT error:" <<  mysql_error(mysql) << '\n';
		return false;
	}
	return true;
}
