from .models import UserInfo
from django import forms


class UserRegisterForm(forms.Form):
    username = forms.CharField(required=True)
    email = forms.EmailField(required=True)
    password = forms.CharField(required=True, min_length=6)


# class ModifyUserInfoForm(forms.ModelForm):
#     class Meta:
#         model = UserInfo
#         fields = ['nickname', 'Email', 'passward', 'Age', 'telephone','address']