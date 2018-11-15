window.onload = function(){

    function timer(){
        var minute = document.getElementById('minute').innerHTML;
        var second = document.getElementById('second').innerHTML;
        var end = false;

        if( second > 0 ) second--;
        else{
            second = 59;

            if( minute > 0 ) minute--;
            else{
                second = 59;
                end = true;
            }
        }

        if(end){
            clearInterval(intervalID);
            minute = 1;
            second = 30;
            timer();
        }else{
            document.getElementById('minute').innerHTML = minute;
            document.getElementById('second').innerHTML = second;
        }
    }
    window.intervalID = setInterval(timer, 1000);
};