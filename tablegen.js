$(document).ready(function() {

//alert('hi');


$('td').hover(function(){
    $('.crosshair').removeClass('crosshair');
    var rowclass = $(this).parent().attr('class'); 
    var fieldclass = $(this).attr('class');

    //$('#diagnostic').html( 'Row = '+rowclass+'<br>Field = '+fieldclass );

    $('tr.'+rowclass+' td, .'+fieldclass).addClass('crosshair');
    $(this).toggleClass('target');

});

$('table').mouseout(function(){
    $('.crosshair').removeClass('crosshair');
});


});
