var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var fs = require('fs');
var questionInterval = 15
var timeLeft = questionInterval;

var questions = []
var raw_questions = fs.readFileSync('questions.txt').toString().split('\n')
for(var q in raw_questions) {
	raw_questions[q] = raw_questions[q].replace('\r', '');
	var parts = raw_questions[q].split('|')
	questions.push({
		'category': parts[1],
		'question': parts[2],
		'answer': parts[3]
	})
}

function editDistance(a, b) {
	memo = new Array(a.length + 1);
	for(var i = 0; i < memo.length; i++) {
		memo[i] = new Array(b.length + 1);
	}

	for(var i = 0; i <= a.length; i++) {
		memo[i][0] = i;
	}

	for(var i = 0; i <= b.length; i++) {
		memo[0][i] = i;
	}

	for(var row = 1; row <= a.length; row++) {
		for(var col = 1; col <= b.length; col++) {
			var result = memo[row-1][col-1] + (a[row - 1] != b[col - 1]);
			if(memo[row-1][col] + 1 < result) {
				result = memo[row-1][col] + 1;
			}
			if(memo[row][col-1] + 1 < result) {
				result = memo[row][col-1] + 1;
			}
			memo[row][col] = result;
		}
	}

	return memo[a.length][b.length];
}

editDistance("gambol", "gumbo");

function get_random_question_index() {
	return Math.floor(Math.random()*questions.length)
}
var question_index = get_random_question_index();
var previous_category = questions[question_index]['category']

function next_question() {
	if(question_index >= 0 && question_index < questions.length)
	{
		var result = questions[question_index]
		if(previous_category == result['category'])
		{
			question_index = question_index + 1
			return result;
		}
		else
		{
			question_index = get_random_question_index();
			previous_category = questions[question_index]['category'];

			while((question_index - 1 > 0) && 
				  (questions[question_index - 1]['category'] == previous_category))
			{
				question_index -= 1;
			}

			return questions[question_index];
		}
	}
	else
	{
		question_index = get_random_question_index();
		return next_question();
	}
}

app.get('/', function(req, res) {
	res.sendfile('client.html');
});

var cur_question = next_question();

var score_board = {};

function get_score_board() {
	function comp(a, b) {
		if(a.score < b.score) return -1;
		else if(a.score > b.score) return 1;
		else return 0;
	}

	parts = [];
	for(var key in score_board) {
		parts.push(score_board[key]);
	}

	parts.sort(comp);

	var result = "\nScoreboard\n=============\n";
	for(var part = parts.length - 1; part >= 0; part--) {
		result += (parts[part].name + "\t:\t" + parts[part].score + "\n");
	}

	return result;
}

io.on('connection', function(socket) {
	console.log('player connected');

	socket.emit('new question', cur_question.category, cur_question.question, timeLeft);

	delete(score_board[socket.id]);
	score_board[socket.id] = {
		name: '',
		score: 0
	};

	socket.on('chat message', function(name, msg) {
		score_board[socket.id]['name'] = name;
		io.emit('chat message', name + ': ' + msg);

		var rawDistance = editDistance(msg, cur_question.answer);
		var normalizedDistance = rawDistance.toFixed(3)/cur_question.answer.length.toFixed(3);
		console.log(normalizedDistance);
		if(normalizedDistance < 0.33) {
			score_board[socket.id]['score'] += 1;
			io.emit('chat message', get_score_board());
			io.emit('chat message', name + ' wins round.');
			timeLeft = 0;
		}
	});

	socket.on('disconnect', function(){
		console.log('user disconnected');
		delete(score_board[socket.id]);
	});
});


setInterval(function() {
	
	if(timeLeft <= 0)
	{
		var prev_question = cur_question;
		io.emit('chat message', 'The correct answer was: ' + prev_question.answer + '.');

		while(prev_question == cur_question) {
			cur_question = next_question();
		}

		console.log(cur_question.answer);

		timeLeft = questionInterval;
		io.emit('new question', cur_question.category, cur_question.question, questionInterval);
	}
	else {
		timeLeft -= 1;
	}

}, 1000);

http.listen(3000, function() {

});