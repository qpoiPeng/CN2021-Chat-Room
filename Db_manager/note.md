
TableName : description
columnName1, columnName2


UserList : 記錄所有的 user
name, password

UserInfo : 你有哪些朋友，在哪些 chatroom
name, friendList (string), chatroomList

FriendRequest : 記錄交友邀請   // from 是關鍵字= =
source, destination

/* if type == user, content is user name, time is last read time */
DirectMessage_[user1]_[user2] :
type (user or message or file), from, content, file, time

Chatroom_[X] :
type (user or message or file), from, content, file, time



---

coding style :
namespace : 全小寫
class struct 名 : 首字大寫，底線分隔
variable : 全小寫，底線分隔
function / method : 全小寫，底線分隔

SQLite 相關 :
TableName
columnName