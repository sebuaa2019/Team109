from django.test import TestCase

class CleanTest(TestCase):
    def test_get(self):
        response = self.client.get('/clean/')
        self.assertEqual(response.status_code, 200)

    def test_post(self):
        response = self.client.post('/clean/', {'level':'1'})
        self.assertEqual(response.status_code, 200)

    def test_err_post(self):
        response = self.client.post('/clean/', {'level':'7'})
        self.assertTrue(b'time should be' in response.content)

    def test_zig_mode_post(self):
        response = self.client.post('/clean/', 
                {'level':'1', 'zig':'Clean Zig-zag'})
        self.assertEqual(response.status_code, 200)



# Create your tests here.
