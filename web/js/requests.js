$(document).ready(function ()
{
    var request = { id: 1, state: 1 };

    $.ajax({
        url: '/',
        type: "post",
        data: JSON.stringify({ command: request }),
        dataType: "json",
        success: function( data ) {
		    //alert( JSON.stringify( data ) );
		    responseT(data);
                },
        failure: function( errMsg ) {
            alert(errMsg);
        }
    });
    function responseT(responseText) {
        var str = JSON.parse(JSON.stringify( responseText ));
        var uuid_h1 = document.getElementById("my_uuid");
        uuid_h1.innerHTML += str["command"]["uuid"];
	uuid_h1.setAttribute("data-shadow", str["command"]["uuid"]);
    }
});