from flask import Flask, render_template, request, redirect
app = Flask(__name__)

leds = [False, False, False, False]

@app.route('/', methods=['GET'])
def root():
    return render_template('index.html')

@app.route('/led/<int:led_id>', methods=['GET', 'POST'])
def led(led_id):

    if not led_id in range(3):
        return '-1'

    if request.method == 'GET':
        return str(leds[led_id])
    
    elif request.method == 'POST':
        leds[led_id] = not leds[led_id]
        print(leds)
        return redirect('/')

@app.route('/ldr', methods=['GET', 'POST'])
def ldr():
    if request.method == 'GET':
        return str(leds[3])

    elif request.method == 'POST':
        print(leds)
        leds[3] = not leds[3]
        return redirect('/')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)