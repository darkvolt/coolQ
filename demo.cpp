#include <cqcppsdk/cqcppsdk.h>
#include <time.h>
#include <../src/core/api.hpp>
#include <../src/core/event.hpp>
#include <iostream>
#include <set>
#include <sstream>
#define random(x) (rand() % x)

using namespace cq;
using namespace std;
using Message = cq::message::Message;
using MessageSegment = cq::message::MessageSegment;
time_t left_time = 0;
int today = 0;
struct grouper *p;
void huangli(const GroupMessageEvent &event);
bool isyesterday(const GroupMessageEvent &event);
struct grouper {
    int id = 0;
    bool did = false;
    string ji;
    string yi;
    string teacher;
    struct grouper *next = NULL;
};
string good[]= {
};
string bad[] ={
};
string qushi[] ={
};
CQ_INIT {
    on_enable([] { logging::info("启用", "插件已启用"); });

    on_private_message([](const PrivateMessageEvent &event) {
        try {
            // send_group_message(, event.message);//把私聊的消息发送到群里
            auto msgid = send_private_message(event.user_id, event.message); // 直接复读消息
            logging::info_success("私聊", "私聊消息复读完成, 消息 Id: " + to_string(msgid));
            send_message(event.target,
                         MessageSegment::face(111) + "这是通过 message 模块构造的消息~"); // 使用 message 模块构造消息
        } catch (ApiError &err) {
            logging::warning("私聊", "私聊消息复读失败, 错误码: " + to_string(err.code));
        }
    });

    on_message([](const MessageEvent &event) {
        logging::debug("消息", "收到消息: " + event.message + "\n实际类型: " + typeid(event).name());
    });

    on_group_message([](const GroupMessageEvent &event) {
        static const set<int64_t> ENABLED_GROUPS = {123456, 1059644011};

        if (ENABLED_GROUPS.count(event.group_id) == 0) return; // 不在启用的群中, 忽略
        try {
            if (event.message == "瓜瓜今日黄历") {
                if (difftime(event.time, left_time) < 2.5) {
                    send_group_message(event.group_id, "请求太频繁瓜瓜罢工了∠( ᐛ 」∠)_");
                } else {
                    left_time = event.time;
                    huangli(event);
                }
                // send_message(event.target, event.message); // 复读
                /*auto mem_list = get_group_member_list(event.group_id); // 获取群成员列表
                string msg;
                for (auto i = 0; i < min(10, static_cast<int>(mem_list.size())); i++) {
                    msg += "昵称: " + mem_list[i].nickname + "\n"; // 拼接前十个成员的昵称
                }*/
                // send_group_message(event.group_id, msg); // 发送群消息
            }
        } catch (ApiError &) { // 忽略发送失败
        }
        if (event.is_anonymous()) {
            logging::info("群聊", "消息是匿名消息, 匿名昵称: " + event.anonymous.name);
        }
        event.block(); // 阻止当前事件传递到下一个插件
    });

    on_group_upload([](const auto &event) { // 可以使用 auto 自动推断类型
        stringstream ss;
        ss << "您上传了一个文件, 文件名: " << event.file.name << ", 大小(字节): " << event.file.size;
        try {
            send_message(event.target, ss.str());
        } catch (ApiError &) {
        }
    });
}

CQ_MENU(menu_demo_1) {
    logging::info("菜单", "点击菜单1");
}

CQ_MENU(menu_demo_2) {
    send_private_message(10000, "测试");
}
void huangli(const GroupMessageEvent &event) {
    struct tm *time_today;
    string luckboy;
    struct grouper *l;
    if (isyesterday(event)) {
        auto mem_list = get_group_member_list(event.group_id); // 获取群成员列表
        auto an = get_group_info(event.group_id); //获取群信息
        srand((int)time(0));
        int i = random(an.member_count);
        luckboy = mem_list[i].card;
        while (p->next) {
            l = p;
            p = p->next;
            free(l);
        }
    } else {
        if (isdid(event)) {
        } else {
            p = new grouper;
            srand((int)time(0));
            int i = random(10);
            p->did = true;
            p->id = event.user_id;
            p->ji = bad[i];
            p->yi = good[i];
            p->teacher = qushi[i];
        }
    }
    send_group_message(event.group_id,
                       ctime(&event.time) +MessageSegment::at(event.user_id) + "的今天\n" + "宜:" + p->yi + "\n"
                           + "忌:" + p->ji + "\n" + "今日幸运角色:" + luckboy + "\n" + "推荐听" + p->teacher
                           + "的曲子画画");
}
bool isyesterday(const GroupMessageEvent &event) {
    struct tm *time_tm = localtime(&event.time);
    if (time_tm->tm_yday > today) {
        today = time_tm->tm_yday;
        send_group_message(event.group_id, "isyesterday");
        return 1;
    } else
        return 0;
}
bool isdid(const GroupMessageEvent &event) {
    while (p->next != NULL) {
        if ( p->id==event.user_id)
            return 1;
        else
            p = p->next;
    }
    if (p->id == event.user_id) return 1;
    return 0;
}