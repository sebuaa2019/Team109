from django.db import models
from datetime import datetime
from django.contrib.auth.models import AbstractUser
# Create your models here.

class UserInfo(AbstractUser):
    user_id = models.AutoField(primary_key=True)
    nick_name = models.CharField(max_length=20, verbose_name=u'昵称', default="")
    Age = models.IntegerField(default=0,verbose_name=u'年龄')
    gender = models.CharField(max_length=10, verbose_name=u'性别', choices=(('male', '男'), ('female', '女')),
                              default="")
    aboutme = models.CharField(max_length=100, verbose_name=u'个人简介',default='')
    mobileNumber = models.CharField(max_length=11, null=True, blank=True, verbose_name=u'电话', default="")
    address = models.CharField(max_length=100, null=True, blank=True, verbose_name=u'地址', default="")
    userImage = models.ImageField(upload_to='image/%Y/%m', max_length=100,
                                  verbose_name=u'头像')

    class Meta:
        verbose_name = '用户信息'
        verbose_name_plural = verbose_name

    def __str__(self):
        return self.username
