from django.test import TestCase

class GrabTest(TestCase):
    def test_get(self):
        response = self.client.get('/grab/')
        self.assertEqual(response.status_code, 200)

    def test_post(self):
        response = self.client.post('/grab/', {'pic':'pic1'})
        self.assertEqual(response.status_code, 200)

    def test_err_post(self):
        response = self.client.post('/grab/', {'pic':'xx'})
        self.assertTrue(b'please choose a label' in response.content)

    def test_zig_mode_post(self):
        response = self.client.post('/grab/',  
                {'pic':'pic1', 'shop':'Go shopping'})
        self.assertEqual(response.status_code, 200)



# Create your tests here.
