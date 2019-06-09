from django.test import TestCase
import json

class GrabTest(TestCase):
    def test_get(self):
        response = self.client.get('/api/move')
        print(response.status_code)
        #self.assertEqual(response.status_code, 200)

    def test_post(self):
        response = self.client.post('/api/move',
                {'json_data': json.dumps({'type':'Turn Left'}) })
        self.assertEqual(response.status_code, 200)

        response = self.client.post('/api/move',
                {'json_data': json.dumps({'type':'Turn Right'}) })
        self.assertEqual(response.status_code, 200)

        response = self.client.post('/api/move',
                {'json_data': json.dumps({'type':'Forwards'}) })
        self.assertEqual(response.status_code, 200)

        response = self.client.post('/api/move',
                {'json_data': json.dumps({'type':'Backwards'}) })
        self.assertEqual(response.status_code, 200)
        
        response = self.client.post('/api/move',
                {'json_data': json.dumps({'type':'Move Right'}) })
        self.assertEqual(response.status_code, 200)
        
        response = self.client.post('/api/move',
                {'json_data': json.dumps({'type':'Move Left'}) })
        self.assertEqual(response.status_code, 200)
        
        response = self.client.post('/api/move',
                {'json_data': json.dumps({'type':'KILL'}) })
        self.assertEqual(response.status_code, 200)

        response = self.client.post('/api/move',
                {'json_data': json.dumps({'type':'Begin'}) })
        self.assertEqual(response.status_code, 200)

        response = self.client.post('/api/move',
                {'json_data': json.dumps({'type':'Stop'}) })
        self.assertEqual(response.status_code, 200)

        response = self.client.post('/api/move',
                {'json_data': json.dumps({'type':'Begin Mapping'}) })
        self.assertEqual(response.status_code, 200)

        response = self.client.post('/api/move',
                {'json_data': json.dumps({'type':'Save Your Map'}) })
        self.assertEqual(response.status_code, 200)

# Create your tests here.
