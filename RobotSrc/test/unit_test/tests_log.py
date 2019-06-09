from django.test import TestCase

class LogTest(TestCase):
    def test_get(self):
        response = self.client.get('/Log/?page=1')
        self.assertEqual(response.status_code, 200)
        response = self.client.get('/Log/?page=x')
        self.assertEqual(response.status_code, 200)
        response = self.client.get('/Log/?page=-1')
        self.assertEqual(response.status_code, 200)
        response = self.client.get('/Log/?page=10000')
        self.assertEqual(response.status_code, 200)
        response = self.client.get('/Log/?page=0000')
        self.assertEqual(response.status_code, 200)

# Create your tests here.
