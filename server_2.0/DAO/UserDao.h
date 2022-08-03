//
// Created by chen on 2022/7/30.
//

#ifndef SERVER_2_0_USERDAO_H
#define SERVER_2_0_USERDAO_H
#include "../sql/sqlConnectionPool.h"
#include <vector>
#include "../protocol/myprotocol.pb.h"
class UserDao {
public:
	UserDao();
	/**
	 * 查询是否存在某个用户
	 * @param pool	数据库连接连接池
	 * @param name	用户名
	 * @param passwd	密码
	 * @param success	是否成功
	 * @return
	 */
	GoBang::Player queryOneUser(connection_pool *pool, const std::string& name, const std::string& passwd, bool &success);
	/**
	 * 查询一些数量的用户
	 * @param pool 数据库连接连接池
	 * @param count 数量 输入 -1 时表示全部
	 * @param start 从多少分开始
	 * @return
	 */
	std::vector<GoBang::Player> querySomeUser(connection_pool *pool, int count, int start);
	/**
	 * 插入一个用户
	 * @param pool	数据库连接连接池
	 * @param name	用户名
	 * @param passwd	密码
	 * @return
	 */
	bool insertUser(connection_pool *pool, const std::string& name, const std::string& passwd);

	bool updateUser(connection_pool *pool, const std::string& uid, int points);
private:
	MYSQL *mysql;
};


#endif //SERVER_2_0_USERDAO_H
