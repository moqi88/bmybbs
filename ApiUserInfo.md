# 用途 #
查询用户的状态信息

# URL #
https://api.bmybbs.com/v1/user/info

# 支持格式 #
JSON

# HTTP请求方法 #
GET

# 是否需要登录 #
否

# 访问授权限制 #
访问级别：普通接口

频次限制：是

# 请求参数 #

|**字段**|**必选**|**类型及范围**|**说明**|
|:---------|:---------|:------------------|:---------|
|id|true|string|用户id|
|appkey|false|string|应用的app key|
|sign|false|string|使用应用私钥生成的签名，包含appkey的时候必填|

# 注意事项 #
PersonalIntro中可能包含彩色代码

# 返回结果 #
```

{
"User":
{
"UserID":"lanboy",
"UserNickName":"GrassRoot-Group",
"LoginCounts":11530,
"PostCounts":54151,
"LastLogin":"Wed Aug 29 10:05:16 2012",
"LastHost":"202.117.7.240",
"BOARDBM":["XJTUannounce","XJTUonecard","BBSipv6","MiM",],
"Job":"本站顾问团",
"States":"null",
"LastLogout":"Wed Aug 29 18:54:06 2012",
"PersonalIntro":"    新浪微博：LANBOY-XJTU     欢迎关注~~~~~\n\n    此人经常位于：北纬3414.9176，东经10858.7734，海拔401.7m的一个桌子上。\n\n    此人长期使用网络，习惯网络化生存。身体移动速度为0，手指移动速度为50m/s，\n        眼球移动速度为500m/s，思维移动速度...无法用设备测量。\n\n    此人习惯用BBS，习惯用数字化语言说教、思考和解决问题。\n\n        此人性格温和，但是已经属于一个老虎。\n        此人善解人意，自制力差的mm不要碰。\n        此人喜欢生活，不爱上进。\n\n",
"Title":"系统管理员",
}
}
```

# 字段说明 #
|**字段**|**类型**|**说明**|
|:---------|:---------|:---------|
|UserID|string|用户ID|
|UserNickName|string|用户昵称|
|LoginCounts|int|上站次数|
|PostCounts|int|文章数目|
|LastLogin|string|上次登录时间|
|LastHost|string|上次访问的ip|
|BORARBM|array|拥有版主权限的版面，非必需|
|Job|string|本站职务，非必需|
|Status|string|本站状态，若当前用户不在线，返回 "null"|
|LastLogout|string|上次离站时间，若当前用户在线，则不显示|
|PersonalIntro|string|个人说明档|
|Title|string|id标记，非必需|

# 其他 #
无