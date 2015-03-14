

# 接口 #
## 版面接口 ##
|**接口**|**说明**|**参考**|**状态**|
|:---------|:---------|:---------|:---------|
|[/board/info](ApiBoardInfo.md)|获取版面信息|bbsdoc|  |
|[/board/list](ApiBoardList.md)|版面文章列表|bbsdoc|  |
|[/board/threadlist](ApiBoardThreadList.md)|版面主题列表|bbstdoc|  |
|[/board/search](ApiBoardSearch.md)|版面搜索|  |  |
|[/board/ban/list](ApiBoardBanList.md)|版面封禁|  |  |
|[/board/ban/add](ApiBoardBanAdd.md)|增加版面封禁|  |  |
|[/board/ban/delete](ApiBoardBanDelete.md)|删除版面封禁|  |  |
|[/board/ban/edit](ApiBoradBanEdit.md)|修改版面封禁|  |  |
|[/board/club/list](ApiBoardClubList.md)|俱乐部版面成员列表|  |  |
|[/board/club/add](ApiBoardClubAdd.md)|增加俱乐部成员|  |  |
|[/board/club/delete](ApiBoardClubDelete.md)|删除俱乐部成员名单|  |  |
|[/board/junk/list](ApiBoardJunkList.md)|垃圾箱文章列表|  |  |
|[/board/junk/resume](ApiBoardJunkResume.md)|垃圾箱文章恢复|  |  |
|[/board/0an/list](ApiBoard0anList.md)|显示路径列表|  |  |

## 收藏夹接口 ##
|**接口**|**说明**|**参考**|**状态**|
|:---------|:---------|:---------|:---------|
|[/favourite/list](ApiFavouriteList.md)|收藏夹版面列表|  |  |
|[/favourite/add](ApiFavouriteAdd.md)|添加版面至收藏夹|  |  |
|[/favourite/delete](ApiFavouriteDelete.md)|删除收藏夹的版面|  |  |

## 分区接口 ##
|**接口**|**说明**|**参考**|**状态**|
|:---------|:---------|:---------|:---------|
|[/section/info](ApiSectionInfo.md)|分区信息|bbsboa|  |

## 文章接口 ##
|**接口**|**说明**|**参考**|**状态**|
|:---------|:---------|:---------|:---------|
|[/article/get](ApiArticleGet.md)|获取文章内容|bbscon|  |
|[/article/edit](ApiArticleEdit.md)|编辑文章内容|  |  |
|[/article/forward](ApiArticleForward.md)|文章转贴/转寄|bbsccc/bbsfwd|  |
|[/article/delete](ApiArticleDelete.md)|删除文章|  |  |
|[/article/post](ApiArticlePost.md)|发表文章|bbspst|  |
|[/article/reply](ApiArticleReply.md)|回复文章|bbspst|  |
|[/article/thread](ApiArticleThread.md)|获取文章主题列表|bbstfind|  |
|[/article/mark](ApiArticleMark.md)|修改文章标注|  |  |

## 信件接口 ##
|**接口**|**说明**|**参考**|**状态**|
|:---------|:---------|:---------|:---------|
|[/mail/list](ApiMailList.md)|获取邮件列表|  |  |
|[/mail/info](ApiMailInfo.md)|邮箱信息|  |  |
|[/mail/send](ApiMailSend.md)|发送邮件|  |  |
|[/mail/reply](ApiMailReply.md)|回复邮件|  |  |
|[/mail/delete](ApiMailDelete.md)|删除邮件|  |  |
|[/mail/forward](ApiMailForward.md)|转发邮件|  |  |
|[/mail/groupsend](ApiMailGroupSend.md)|群发邮件|  |  |

## 附件接口 ##
|**接口**|**说明**|**参考**|**状态**|
|:---------|:---------|:---------|:---------|
|[/attach/get](ApiAttachGet.md)|获取附件|  |  |
|[/attach/upload](ApiAttachUpload.md)|上传附件|  |  |
|[/attach/delete](ApiAttachDelete.md)|删除附件|  |  |

## 消息接口 ##
|**接口**|**说明**|**参考**|**状态**|
|:---------|:---------|:---------|:---------|

## 用户接口 ##
|**接口**|**说明**|**参考**|**状态**|
|:---------|:---------|:---------|:---------|
|[/user/info](ApiUserInfo.md)|用户信息|bbsqry|  |
|[/user/login](ApiUserLogin.md)|用户登录|  |  |
|[/user/logout](ApiUserLogout.md)|用户退出|  |  |

## 好友接口 ##
|**接口**|**说明**|**参考**|**状态**|
|:---------|:---------|:---------|:---------|
|[/friend/list](ApiFriendList.md)|好友列表|  |  |
|[/friend/add](ApiFriendAdd.md)|添加好友|  |  |
|[/friend/delete](ApiFriendDelete.md)|删除好友|  |  |

## 元数据 ##
|**名称**|**说明**|**参考**|**状态**|
|:---------|:---------|:---------|:---------|

# 备注 #
## 接口模板 ##
```
#summary 接口描述
#sidebar TableOfContents
#labels API

=用途=

=URL=

=支持格式=

=HTTP请求方法=

=是否需要登录=

=访问授权限制=

=请求参数=
||*字段*||*必选*||*类型及范围*||*说明*||

=注意事项=

=返回结果=

=字段说明=
||*字段*||*类型*||*说明*||

=其他=
```

## 变更日志 ##
|**日期**|**版本**|**作者**|**说明**|
|:---------|:---------|:---------|:---------|
|2012/08/29|v0.0.1|IronBlood|initial ci|
|2012/08/30|v0.0.2|IronBlood|接口文档模板中加入说明表格|