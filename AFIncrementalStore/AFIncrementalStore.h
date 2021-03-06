// AFIncrementalStore.h
//
// Copyright (c) 2012 Mattt Thompson (http://mattt.me)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#import <CoreData/CoreData.h>
#import "AFNetworking.h"

@protocol AFIncrementalStoreDelegate;
@protocol AFIncrementalStoreHTTPClient;

/**
 `AFIncrementalStore` is an abstract subclass of `NSIncrementalStore`, designed to allow you to load and save data incrementally to and from a one or more web services.
 
 ## Subclassing Notes
 
 ### Methods to override
 
 In a subclass of `AFIncrementalStore`, you _must_ override the following methods to provide behavior appropriate for your store:
    
    - +type
    - +model
 
 Additionally, all `NSPersistentStore` subclasses, and thus all `AFIncrementalStore` subclasses must do `NSPersistentStoreCoordinator +registerStoreClass:forStoreType:` in order to be created by `NSPersistentStoreCoordinator -addPersistentStoreWithType:configuration:URL:options:error:`. It is recommended that subclasses register themselves in their own `+initialize` method.
 */
@interface AFIncrementalStore : NSIncrementalStore

///---------------------------------------------
/// @name Accessing Incremental Store Properties
///---------------------------------------------

/**
 The HTTP client used to manage requests and responses with the associated web services.
 */
@property (nonatomic, strong) AFHTTPClient <AFIncrementalStoreHTTPClient> *HTTPClient;

/**
 The persistent store coordinator used to persist data from the associated web serivices locally.
 
 @discussion Rather than persist values directly, `AFIncrementalStore` manages and proxies through a persistent store coordinator.
 */
@property (readonly) NSPersistentStoreCoordinator *backingPersistentStoreCoordinator;

///-----------------------
/// @name Required Methods
///-----------------------

/**
 Returns the string used as the `NSStoreTypeKey` value by the application's persistent store coordinator.
 
 @return The string used to describe the type of the store.
 */
+ (NSString *)type;

/**
 Returns the managed object model used by the store.
 
 @return The managed object model used by the store
 */
+ (NSManagedObjectModel *)model;

@end

#pragma mark -

/**
 The `AFIncrementalStoreHTTPClient` protocol defines the methods used by the HTTP client to interract with the associated web services of an `AFIncrementalStore`.
 */
@protocol AFIncrementalStoreHTTPClient <NSObject>

///-----------------------
/// @name Required Methods
///-----------------------

/**
 Returns an `NSDictionary` or an `NSArray` of `NSDictionaries` containing the representations of the resources found in a response object.
 
 @discussion For example, if `GET /users` returned an `NSDictionary` with an array of users keyed on `"users"`, this method would return the keyed array. Conversely, if `GET /users/123` returned a dictionary with all of the atributes of the requested user, this method would simply return that dictionary.

 @param responseObject The response object returned from the server.
 
 @return An `NSDictionary` or an `NSArray` of `NSDictionaries` containing the resource representations.
 */
- (id)representationOrArrayOfRepresentationsFromResponseObject:(id)responseObject;

/**
 Returns an `NSDictionary` containing the representations of associated objects found within the representation of a response object, keyed by their relationship name.
 
 @discussion For example, if `GET /albums/123` returned the representation of an album, including the tracks as sub-entities, keyed under `"tracks"`, this method would return a dictionary with an array of representations for those objects, keyed under the name of the relationship used in the model (which is likely also to be `"tracks"`). Likewise, if an album also contained a representation of its artist, that dictionary would contain a dictionary representation of that artist, keyed undrer the name of the relationship used in the model (which is likely also to be `"artist"`).
 
 @param representation The resource representation.
 @param entity The entity for the representation.
 @param response The HTTP response for the resource request.
 
 @return An `NSDictionary` containing representations of relationships, keyed by relationship name.
 */
- (NSDictionary *)representationsForRelationshipsFromRepresentation:(NSDictionary *)representation
                                                           ofEntity:(NSEntityDescription *)entity
                                                       fromResponse:(NSHTTPURLResponse *)response;

/**
 Returns the resource identifier for the resource whose representation of an entity came from the specified HTTP response. A resource identifier is a string that uniquely identifies a particular resource. If new attributes come back for an existing resource identifier, the managed object associated with that resource identifier will be updated, rather than a new object being created.
 
 @discussion For example, if `GET /posts` returns a collection of posts, the resource identifier for any particular one might be its URL-safe "slug" or parameter string, or perhaps its numeric id.
 
 @param representation The resource representation.
 @param entity The entity for the representation.
 @param response The HTTP response for the resource request.
 
 @return An `NSString` resource identifier for the resource.
 */
- (NSString *)resourceIdentifierForRepresentation:(NSDictionary *)representation
                                         ofEntity:(NSEntityDescription *)entity
                                     fromResponse:(NSHTTPURLResponse *)response;

/**
 Returns the attributes for the managed object corresponding to the representation of an entity from the specified response. This method is used to get the attributes of the managed object from its representation returned in `-representationOrArrayOfRepresentationsFromResponseObject` or `representationsForRelationshipsFromRepresentation:ofEntity:fromResponse:`.
 
 @discussion For example, if the representation returned from `GET /products/123` had a `description` field that corresponded with the `productDescription` attribute in its Core Data model, this method would set the value of the `productDescription` key in the returned dictionary to the value of the `description` field in representation.
 
 @param representation The resource representation.
 @param entity The entity for the representation.
 @param response The HTTP response for the resource request.
 
 @return An `NSDictionary` containing the attributes for a managed object. 
 */
- (NSDictionary *)attributesForRepresentation:(NSDictionary *)representation
                                     ofEntity:(NSEntityDescription *)entity
                                 fromResponse:(NSHTTPURLResponse *)response;

/**
 Returns a URL request object for the specified fetch request within a particular managed object context.
 
 @discussion For example, if the fetch request specified the `User` entity, this method might return an `NSURLRequest` with `GET /users` if the web service was RESTful, `POST /endpoint?method=users.getAll` for an RPC-style system, or a request with an XML envelope body for a SOAP webservice.
 
 @param fetchRequest The fetch request to translate into a URL request.
 @param context The managed object context executing the fetch request.
 
 @return An `NSURLRequest` object corresponding to the specified fetch request.
 */
- (NSURLRequest *)requestForFetchRequest:(NSFetchRequest *)fetchRequest
                             withContext:(NSManagedObjectContext *)context;

/**
 Returns a URL request object with a given HTTP method for a particular managed object. This method is used in `AFIncrementalStore -newValuesForObjectWithID:withContext:error`.
 
 @discussion For example, if a `User` managed object were to be refreshed, this method might return a `GET /users/123` request.
 
 @param method The HTTP method of the request.
 @param objectID The object ID for the specified managed object.
 @param context The managed object context for the managed object.
 
 @return An `NSURLRequest` object with the provided HTTP method for the resource corresponding to the managed object.
 */
- (NSURLRequest *)requestWithMethod:(NSString *)method
                pathForObjectWithID:(NSManagedObjectID *)objectID
                        withContext:(NSManagedObjectContext *)context;

/**
 Returns a URL request object with a given HTTP method for a particular relationship of a given managed object. This method is used in `AFIncrementalStore -newValueForRelationship:forObjectWithID:withContext:error:`.
 
 @discussion For example, if a `Department` managed object was attempting to fulfill a fault on the `employees` relationship, this method might return `GET /departments/sales/employees`.
 
 @param method The HTTP method of the request.
 @param relationship The relationship of the specifified managed object 
 @param objectID The object ID for the specified managed object.
 @param context The managed object context for the managed object.
 
 @return An `NSURLRequest` object with the provided HTTP method for the resource or resoures corresponding to the relationship of the managed object.

 */
- (NSURLRequest *)requestWithMethod:(NSString *)method
                pathForRelationship:(NSRelationshipDescription *)relationship
                    forObjectWithID:(NSManagedObjectID *)objectID
                        withContext:(NSManagedObjectContext *)context;

@optional

///-----------------------
/// @name Optional Methods
///-----------------------

/**
 Returns whether the client should fetch remote attribute values for a particular managed object. This method is consulted when a managed object faults on an attribute, and will call `-requestWithMethod:pathForObjectWithID:withContext:` if `YES`.
 
 @param objectID The object ID for the specified managed object.
 @param context The managed object context for the managed object.
 
 @return `YES` if an HTTP request should be made, otherwise `NO.
 */
- (BOOL)shouldFetchRemoteAttributeValuesForObjectWithID:(NSManagedObjectID *)objectID
                                 inManagedObjectContext:(NSManagedObjectContext *)context;

/**
 Returns whether the client should fetch remote relationship values for a particular managed object. This method is consulted when a managed object faults on a particular relationship, and will call `-requestWithMethod:pathForRelationship:forObjectWithID:withContext:` if `YES`.
 
 @param relationship The relationship of the specifified managed object
 @param objectID The object ID for the specified managed object.
 @param context The managed object context for the managed object.
 
 @return `YES` if an HTTP request should be made, otherwise `NO.
 */
- (BOOL)shouldFetchRemoteValuesForRelationship:(NSRelationshipDescription *)relationship
                               forObjectWithID:(NSManagedObjectID *)objectID
                        inManagedObjectContext:(NSManagedObjectContext *)context;

@end


///----------------
/// @name Constants
///----------------

/**
 The name of the exception called when `AFIncrementalStore` or a subclass is attempted to be used, without implementing one of the required methods.
 */
extern NSString * AFIncrementalStoreUnimplementedMethodException;

