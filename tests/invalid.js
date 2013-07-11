var assert = require('assert');
var parser = require('../');

describe('Fast-feed', function() {

    it('should throw on invalid input', function() {
        try {
            parser.parse('<<<<>');
        } catch (err) {
            assert.ok(err);
        }
    });

    it('should set error on invalid input', function(done) {
        parser.parse('<<<<>', function(err) {
            assert.ok(err);
            done();
        });
    });
});