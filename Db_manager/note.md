
TableName : description
columnName1, columnName2


UserList : 記錄所有的 user
name, password

UserInfo : 你有哪些朋友，在哪些 chatroom
name, friendList (string), chatroomList

FriendRequest : 記錄交友邀請
from, to

/* if type == user, content is user name, time is last read time */
DirectMessage_[user1]_[user2] :
type (user or message or file), from, content, file, time

Chatroom_[X] :
type (user or message or file), from, content, file, time

