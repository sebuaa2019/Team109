from django.db import models

# Create your models here.

class logs(models.Model):
    time = models.CharField(max_length=30, default="")
    type = models.CharField(max_length=10, default="")
    info = models.CharField(max_length=100, default="")
